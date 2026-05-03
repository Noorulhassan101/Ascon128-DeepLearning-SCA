# ASCON-128 Study Report

## 1. Overview of ASCON-128
ASCON is a family of lightweight authenticated encryption and hashing algorithms. ASCON-128 specifically is the primary choice for lightweight authenticated encryption with associated data (AEAD) in the CAESAR competition and has been selected by NIST as the standard for lightweight cryptography. It is designed to be highly efficient in hardware and software, especially on constrained devices.

## 2. AEAD Explanation
Authenticated Encryption with Associated Data (AEAD) provides both confidentiality (encryption) and authenticity/integrity for a message. It takes a plaintext, a secret key, a nonce, and associated data (which is authenticated but not encrypted). The output is a ciphertext and an authentication tag. If an attacker modifies the ciphertext or the associated data, the tag verification will fail, ensuring that the receiver can detect any tampering.

## 3. Sponge Construction
ASCON is based on a sponge construction. It consists of an internal state that undergoes a permutation function repeatedly. The construction operates in two phases:
- **Absorbing:** Data (associated data, plaintext) is XORed into a part of the state (the rate).
- **Squeezing:** Data (ciphertext, tag) is extracted from the rate part of the state.
The unexposed part of the state (the capacity) ensures the cryptographic security of the design.

## 4. 320-bit State Explanation
The internal state of ASCON-128 is 320 bits, divided into five 64-bit words: $x_0, x_1, x_2, x_3, x_4$. 
- The **rate** ($r$) is 64 bits (for ASCON-128), which means 64 bits of data are absorbed or squeezed per iteration.
- The **capacity** ($c$) is 256 bits, providing the necessary security margin.

## 5. Permutation Rounds
The core of ASCON is its permutation $p$, which operates on the 320-bit state. It uses 12 rounds ($p^{12}$) for initialization and finalization, and 6 rounds ($p^6$) or 8 rounds ($p^8$) for intermediate data processing. Each round consists of three steps:
1. Addition of Round Constants ($p_C$)
2. Substitution Layer ($p_S$)
3. Linear Diffusion Layer ($p_L$)

### 5.1. Round Constants ($p_C$)
In each round, a specific 8-bit round constant is XORed with the word $x_2$. This destroys the symmetry between the rounds and prevents slide attacks.

### 5.2. S-box Operation ($p_S$)
The substitution layer uses a custom 5-bit S-box applied 64 times in parallel across the five 64-bit words. For a specific bit position $i$, the 5 bits from $x_0[i], x_1[i], x_2[i], x_3[i], x_4[i]$ form the input to the S-box, and the output updates these 5 bits. The S-box is designed to provide non-linearity while being highly efficient to implement using bitwise logical operations (AND, NOT, XOR).

### 5.3. Linear Diffusion ($p_L$)
The linear diffusion layer provides mixing within each of the five 64-bit words independently. Each word $x_i$ is updated by XORing it with two circularly shifted versions of itself:
$x_i = x_i \oplus (x_i \ggg r_{i,1}) \oplus (x_i \ggg r_{i,2})$
This ensures that any single-bit difference quickly diffuses across the entire 64-bit word.

## 6. Potential Side-Channel Leakage Points
Side-channel attacks (like Power Analysis or Electromagnetic Analysis) exploit the physical leakage of the device during the execution of cryptographic algorithms. In ASCON-128, the critical points of leakage typically occur during operations involving the secret key or state variables dependent on the secret key.

**Key Leakage Points:**
1. **First Round S-box:** During the first round of encryption (after initialization), the plaintext is XORed with the state (which contains the key-dependent initialization output). The non-linear S-box operation following this state update is a prime target for side-channel attacks because the power consumption is highly correlated with the Hamming Weight (HW) of the intermediate values being processed.
2. **Linear Layer Mixing:** While linear operations leak less distinct information compared to non-linear ones, the shifts and XORs in the linear diffusion layer also cause power variations that can be profiled.
3. **Initialization Phase:** The absorption of the key during the initial $p^{12}$ permutation could leak information about the key, though attackers typically prefer the more controlled environment of the first encryption round where the plaintext is known and varies.

For this project, we will target the output of the S-box operation during the first round of data encryption, utilizing a Hamming Weight leakage model.
