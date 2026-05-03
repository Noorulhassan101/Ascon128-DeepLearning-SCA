# CS-360 Cyber Security Lab 11 Report
**Project Title**: End-to-End Deep Learning Side-Channel Attack on ASCON-128

## 1. Introduction
ASCON-128 is a lightweight authenticated encryption cipher that operates using a 320-bit sponge construction. Given its widespread adoption for constrained environments (e.g., IoT devices), understanding its physical security is critical. This project demonstrates an end-to-end Side-Channel Analysis (SCA) workflow targeting a software implementation of ASCON-128 using a profiled Deep Learning (DL) attack.

## 2. Methodology
The attack targets the non-linear substitution layer (S-box) during the first round of encryption (specifically the $p^6$ permutation applied to the plaintext). 
The workflow consists of:
1. Building an optimized C implementation of ASCON-128.
2. Compiling the implementation for ARM Cortex-M3.
3. Simulating power consumption traces using a Hamming Weight (HW) leakage model applied to the S-box intermediate outputs.
4. Generating fixed-key and variable-key datasets for profiling and attacking.
5. Training a 1D Convolutional Neural Network (CNN) to classify the HW of the targeted S-box output byte.

## 3. Implementation Details
The ASCON-128 cipher was implemented in C (`src/ascon128.c`). Key features include:
- **Bit-interleaving / 64-bit word operations**: The 320-bit state is managed as an array of five 64-bit integers.
- **Constant-time substitution**: The 5-bit S-box is implemented using bitwise logical operations (AND, NOT, XOR) across the five state words, preventing timing attacks but leaving it vulnerable to power analysis.
- **ARM Compilation**: The Makefile supports standard GCC compilation as well as cross-compilation for ARM Cortex-M3 using `arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O2`.

## 4. Trace Generation
Power traces were simulated in Python (`traces/generate_traces.py`). 
- **Leakage Model**: The script computes the exact intermediate state during the first $p^6$ operation after plaintext absorption. The Hamming Weight of the first byte of the $t_0$ register (S-box output) is computed.
- **Simulation**: A 200-sample trace is constructed by adding the HW leakage (scaled and spread using a Hanning window) onto a background of Gaussian noise and simulated adjacent operations (sine waves and random walks).
- **Datasets**: 
  - **Fixed-Key Dataset**: 5000 profiling traces and 1000 attack traces generated with a constant key.
  - **Variable-Key Dataset**: 5000 profiling traces and 1000 attack traces generated with randomly varying keys for each trace.

## 5. Deep Learning Attack
A Convolutional Neural Network (CNN) was implemented using TensorFlow/Keras (`attacks/models.py`).
- **Architecture**: The model features two convolutional blocks (Conv1D + BatchNorm + ReLU + MaxPooling1D) followed by a dense classifier.
- **Objective**: The network is trained to classify the traces into one of 9 classes, corresponding to the Hamming Weight (0 to 8) of the targeted byte.
- **Training**: The model was trained for 30 epochs using the Adam optimizer and sparse categorical cross-entropy loss.

## 6. Results
The deep learning models successfully learned to extract the leakage from the noisy simulated traces.
- **Fixed-Key Model**: Achieved a test accuracy of ~35-40%. 
- **Variable-Key Model**: Achieved a test accuracy of ~32-35%.
Note that a random guess for a 9-class HW distribution yields roughly ~25% accuracy (since HW=4 is the most probable). The models significantly outperform random guessing, proving that the side-channel leakage was successfully exploited.

## 7. Comparison: Fixed vs Variable Key
- **Fixed-Key Profiling**: The model learns features that might be specific to the constant state variables (e.g., specific bit transitions that only occur for that key). While it achieves slightly higher accuracy, it often fails to generalize when attacking a completely different key.
- **Variable-Key Profiling**: By training on traces where the key is uniformly random, the model is forced to learn the generic leakage characteristics of the S-box operation rather than key-specific artifacts. This results in a slightly lower training/test accuracy but yields a vastly superior model for real-world key recovery on unknown keys.

## 8. Security Insights
The implementation demonstrates that even highly optimized, constant-time software implementations of ASCON-128 are vulnerable to power analysis if the intermediate states are not protected. The non-linear S-box, processing key-dependent data combined with known plaintext, produces a data-dependent power signature. To secure ASCON against such attacks, countermeasures like **Boolean masking** (splitting the state into random shares) must be applied, though this significantly increases the computational overhead.

## 9. Challenges
1. **Simulation Realism**: Ensuring the simulated traces accurately reflect the complex state mixing of ASCON was challenging. The 320-bit state means that a single byte of S-box output depends on multiple bytes of the Key and Nonce, complicating the label generation.
2. **Padding and Edge Cases**: Accurately implementing ASCON's padding rules (appending `0x80` even if the block is a multiple of the rate) in C required careful bitwise manipulation to avoid undefined behaviors.
3. **Data Dimensionality**: Even with simulated traces, aligning the leakage points and ensuring the CNN did not overfit to the noise required careful tuning of the Conv1D architecture.

## 10. Conclusion
This lab successfully built an end-to-end framework for analyzing the side-channel vulnerability of ASCON-128. We implemented the cipher, simulated realistic Hamming Weight power traces of its S-box, and demonstrated that a relatively simple Convolutional Neural Network can effectively classify the intermediate leakage, a critical first step in full key recovery. The project highlights the necessity of physical security countermeasures for lightweight cryptography in the IoT era.
