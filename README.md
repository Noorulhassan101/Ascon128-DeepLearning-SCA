# 🛡️ ASCON-128 Side-Channel Analysis (SCA) with Deep Learning

![ASCON-128](https://img.shields.io/badge/Cipher-ASCON--128-blueviolet?style=for-the-badge)
![Python](https://img.shields.io/badge/Python-3.8+-blue?style=for-the-badge&logo=python)
![TensorFlow](https://img.shields.io/badge/TensorFlow-2.x-orange?style=for-the-badge&logo=tensorflow)
![License](https://img.shields.io/badge/Security-Lab%20Project-red?style=for-the-badge)

An end-to-end practical framework for performing side-channel analysis on the **ASCON-128** lightweight authenticated encryption cipher. This project simulates power consumption leakage and utilizes **Convolutional Neural Networks (CNNs)** to perform a profiled key recovery attack.

---

## 🚀 Overview

This repository contains the complete workflow for **CS-360 Cyber Security Lab 11**. It demonstrates how physical leakage from a device (simulated via Hamming Weight) can be exploited using modern Deep Learning techniques to recover secret keys.

### 🔑 Key Phases
1.  **Cipher Implementation**: ASCON-128 in C, optimized for ARM Cortex-M3.
2.  **Trace Simulation**: Generation of noisy power traces using a physical leakage model.
3.  **Deep Learning Attack**: Training a 1D-CNN to classify secret intermediate values.
4.  **Key Recovery**: Performing rank analysis to extract the secret key.

---

## 📂 Project Structure

```text
lab11_ascon/
├── phase1_report/      # ASCON-128 study & theoretical analysis
├── src/                # ASCON-128 C source code & Makefile
├── traces/             # Trace simulation scripts & HDF5 datasets
├── attacks/            # CNN models, utils, and attack scripts
├── results/            # Training curves, key ranks, and screenshots
├── models/             # Trained .h5 model files
└── README.md           # Project documentation
```

---

## 🛠️ Prerequisites

Ensure you have the following installed:
*   **Python 3.8+**
*   **GCC** (for native testing)
*   **ARM GCC Toolchain** (optional, for `ascon128.elf`)
*   **Python Libraries**: `numpy`, `h5py`, `matplotlib`, `tensorflow`

Install dependencies:
```bash
pip install numpy h5py matplotlib tensorflow
```

---

## 🏃 How to Run

### Step 1: Compile the Cipher
```bash
cd src
gcc ascon128.c test_vectors.c -o ascon_test
./ascon_test
cd ..
```

### Step 2: Generate Power Traces
```bash
python traces/generate_traces.py
```
*Creates `fixed_key.h5` and `variable_key.h5` datasets with 6,000 traces each.*

### Step 3: Train & Attack
```bash
cd attacks
python attack_fixed_key.py
python attack_variable_key.py
```

### Step 4: Key Rank Analysis
```bash
python key_rank_plot.py
```

---

## 📊 Methodology & Results

### Side-Channel Leakage
We target the **S-box substitution layer** during the first round of encryption. The power consumption is modeled using the **Hamming Weight (HW)** of the intermediate state:
$$P_{total} \propto HW(State_{intermediate}) + Noise$$

### CNN Architecture
We use a 1D Convolutional Neural Network designed to handle noise and misalignment in power traces:
*   **Conv1D Layers**: For automatic feature extraction.
*   **BatchNormalization**: For training stability.
*   **Softmax Output**: To predict the probability of each HW class (0-8).

### Results
The model successfully recovers the secret key byte (0x00) within **~450 traces**, achieving a significant accuracy boost over the random guessing baseline.

---

## 📝 Reports
*   [Phase 1: ASCON Study Report](phase1_report/ascon_study_report.md)
*   [Final: Lab 11 Project Report](final_report/lab11_report.md)

---

## 🎓 Credits
Developed for **CS-360 Cyber Security Lab 11: Side-Channel Analysis**.
