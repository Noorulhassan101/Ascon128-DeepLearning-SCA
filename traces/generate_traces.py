import os
import numpy as np
import h5py
import matplotlib.pyplot as plt

# Ensure traces directory and subdirectories exist
os.makedirs("traces/trace_plots", exist_ok=True)

# ASCON-128 Constants
IV = 0x80400c0600000000
RC = [0xf0, 0xe1, 0xd2, 0xc3, 0xb4, 0xa5, 0x96, 0x87, 0x78, 0x69, 0x5a, 0x4b]

def rotr(val, r):
    return ((val >> r) | (val << (64 - r))) & 0xFFFFFFFFFFFFFFFF

def ascon_permutation(s, rounds):
    start_round = 12 - rounds
    for r in range(start_round, 12):
        s[2] ^= RC[r]
        
        x0, x1, x2, x3, x4 = s[0], s[1], s[2], s[3], s[4]
        x0 ^= x4; x4 ^= x3; x2 ^= x1
        
        t0 = x0 ^ ((~x1) & x2) & 0xFFFFFFFFFFFFFFFF
        t1 = x1 ^ ((~x2) & x3) & 0xFFFFFFFFFFFFFFFF
        t2 = x2 ^ ((~x3) & x4) & 0xFFFFFFFFFFFFFFFF
        t3 = x3 ^ ((~x4) & x0) & 0xFFFFFFFFFFFFFFFF
        t4 = x4 ^ ((~x0) & x1) & 0xFFFFFFFFFFFFFFFF
        
        t1 ^= t0; t0 ^= t4; t3 ^= t2; t2 = (~t2) & 0xFFFFFFFFFFFFFFFF
        
        s[0] = t0 ^ rotr(t0, 19) ^ rotr(t0, 28)
        s[1] = t1 ^ rotr(t1, 61) ^ rotr(t1, 39)
        s[2] = t2 ^ rotr(t2, 1) ^ rotr(t2, 6)
        s[3] = t3 ^ rotr(t3, 10) ^ rotr(t3, 17)
        s[4] = t4 ^ rotr(t4, 7) ^ rotr(t4, 41)

def get_intermediate_sbox_output(key_bytes, nonce_bytes, pt_bytes):
    # Initialize
    k0 = int.from_bytes(key_bytes[:8], 'big')
    k1 = int.from_bytes(key_bytes[8:], 'big')
    n0 = int.from_bytes(nonce_bytes[:8], 'big')
    n1 = int.from_bytes(nonce_bytes[8:], 'big')
    
    s = [IV, k0, k1, n0, n1]
    ascon_permutation(s, 12)
    s[3] ^= k0
    s[4] ^= k1
    
    # Process AD (Assuming none for simplicity)
    s[4] ^= 1
    
    # Process Plaintext (first block)
    pt = int.from_bytes(pt_bytes[:8], 'big')
    s[0] ^= pt
    
    # Now we simulate the first step of the permutation (S-box)
    # 1. Addition of Constants (r=6 for p^6)
    s[2] ^= RC[12 - 6]
    
    # 2. Substitution Layer
    x0, x1, x2, x3, x4 = s[0], s[1], s[2], s[3], s[4]
    x0 ^= x4; x4 ^= x3; x2 ^= x1
    
    t0 = x0 ^ ((~x1) & x2) & 0xFFFFFFFFFFFFFFFF
    t1 = x1 ^ ((~x2) & x3) & 0xFFFFFFFFFFFFFFFF
    t2 = x2 ^ ((~x3) & x4) & 0xFFFFFFFFFFFFFFFF
    t3 = x3 ^ ((~x4) & x0) & 0xFFFFFFFFFFFFFFFF
    t4 = x4 ^ ((~x0) & x1) & 0xFFFFFFFFFFFFFFFF
    
    t1 ^= t0; t0 ^= t4; t3 ^= t2; t2 = (~t2) & 0xFFFFFFFFFFFFFFFF
    
    # We return the first byte of t0 as our leakage target
    return (t0 >> 56) & 0xFF

def hw(val):
    return bin(val).count("1")

def generate_traces(num_traces, fixed_key=True):
    traces = np.zeros((num_traces, 200), dtype=np.float32)
    plaintexts = np.zeros((num_traces, 16), dtype=np.uint8)
    keys = np.zeros((num_traces, 16), dtype=np.uint8)
    labels = np.zeros(num_traces, dtype=np.uint8)
    
    fixed_k = os.urandom(16)
    fixed_n = os.urandom(16)
    
    for i in range(num_traces):
        if fixed_key:
            k = fixed_k
        else:
            k = os.urandom(16)
            
        pt = os.urandom(16)
        
        plaintexts[i] = np.frombuffer(pt, dtype=np.uint8)
        keys[i] = np.frombuffer(k, dtype=np.uint8)
        
        # Intermediate value
        target_byte = get_intermediate_sbox_output(k, fixed_n, pt)
        leakage = hw(target_byte)
        
        # Simulate trace
        trace = np.random.normal(0, 1.5, 200) # Noise
        
        # Add leakage at specific points (e.g., sample 50)
        trace[45:55] += np.hanning(10) * (leakage * 0.8)
        
        # Add some other "operations" to make it look realistic
        trace[10:30] += np.sin(np.linspace(0, 4*np.pi, 20)) * 2
        trace[100:150] += np.random.normal(0, 0.5, 50)
        
        traces[i] = trace
        labels[i] = leakage
        
    return traces, plaintexts, keys, labels

def save_dataset(filename, traces_prof, pt_prof, keys_prof, labels_prof, traces_att, pt_att, keys_att, labels_att):
    with h5py.File(filename, 'w') as f:
        prof = f.create_group("Profiling_traces")
        prof.create_dataset("traces", data=traces_prof)
        prof.create_dataset("metadata", data=pt_prof) # Using metadata for plaintexts
        prof.create_dataset("keys", data=keys_prof)
        prof.create_dataset("labels", data=labels_prof)
        
        att = f.create_group("Attack_traces")
        att.create_dataset("traces", data=traces_att)
        att.create_dataset("metadata", data=pt_att)
        att.create_dataset("keys", data=keys_att)
        att.create_dataset("labels", data=labels_att)

def main():
    print("Generating Fixed-Key Dataset...")
    traces_prof_f, pt_prof_f, keys_prof_f, labels_prof_f = generate_traces(5000, fixed_key=True)
    traces_att_f, pt_att_f, keys_att_f, labels_att_f = generate_traces(1000, fixed_key=True)
    save_dataset("traces/fixed_key.h5", traces_prof_f, pt_prof_f, keys_prof_f, labels_prof_f, traces_att_f, pt_att_f, keys_att_f, labels_att_f)
    
    print("Generating Variable-Key Dataset...")
    traces_prof_v, pt_prof_v, keys_prof_v, labels_prof_v = generate_traces(5000, fixed_key=False)
    traces_att_v, pt_att_v, keys_att_v, labels_att_v = generate_traces(1000, fixed_key=False)
    save_dataset("traces/variable_key.h5", traces_prof_v, pt_prof_v, keys_prof_v, labels_prof_v, traces_att_v, pt_att_v, keys_att_v, labels_att_v)
    
    print("Plotting Sample Traces...")
    plt.figure(figsize=(10, 6))
    for i in range(10):
        plt.plot(traces_prof_f[i], alpha=0.7)
    plt.title("Simulated Power Traces of ASCON-128 S-box")
    plt.xlabel("Time Samples")
    plt.ylabel("Power Consumption (a.u.)")
    plt.grid(True)
    plt.savefig("traces/trace_plots/sample_traces.png")
    plt.close()
    
    print("Dataset generation complete!")

if __name__ == "__main__":
    main()
