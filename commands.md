# ASCON-128 Side-Channel Project: Terminal Commands

## 1. Compilation
Navigate to the `src` directory and compile the ASCON implementation:
```powershell
cd src
# Native compilation for testing
gcc ascon128.c test_vectors.c -o ascon_test
# ARM Cortex-M3 compilation (requires arm-none-eabi-gcc)
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O2 -Wall -Wextra --specs=nosys.specs ascon128.c test_vectors.c -o ascon128.elf
```

## 2. Trace Generation
Generate the power trace datasets (Fixed-Key and Variable-Key):
```powershell
cd ..
python traces/generate_traces.py
```

## 3. Deep Learning Attacks
Train the CNN models and perform the attacks:
```powershell
cd attacks
# Attack Fixed-Key Dataset
python attack_fixed_key.py
# Attack Variable-Key Dataset
python attack_variable_key.py
```

## 4. Viewing Results
Check the generated reports and plots:
- Reports: `phase1_report/ascon_study_report.md`, `final_report/lab11_report.md`
- Traces: `traces/trace_plots/`
- Training Curves: `results/training_curves/`
- Compilation Logs: `src/execution_log.txt`
