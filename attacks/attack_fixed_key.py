import os
import numpy as np
from utils import load_dataset, plot_training_history
from models import create_cnn_model

def main():
    print("Loading Fixed-Key Dataset...")
    traces_prof, pt_prof, keys_prof, labels_prof, traces_att, pt_att, keys_att, labels_att = load_dataset("../traces/fixed_key.h5")
    
    # Reshape traces for CNN input (num_traces, num_samples, 1)
    X_train = np.expand_dims(traces_prof, axis=2)
    y_train = labels_prof
    
    X_test = np.expand_dims(traces_att, axis=2)
    y_test = labels_att
    
    print(f"X_train shape: {X_train.shape}, y_train shape: {y_train.shape}")
    print(f"X_test shape: {X_test.shape}, y_test shape: {y_test.shape}")
    
    model = create_cnn_model(input_length=200, num_classes=9)
    model.summary()
    
    print("Training CNN on Fixed-Key Traces...")
    history = model.fit(
        X_train, y_train,
        epochs=30,
        batch_size=64,
        validation_data=(X_test, y_test),
        verbose=1
    )
    
    os.makedirs("../models", exist_ok=True)
    os.makedirs("../results/training_curves", exist_ok=True)
    os.makedirs("../results/key_rank_analysis", exist_ok=True)
    
    model.save("../models/model_fixed_key.h5")
    plot_training_history(history, "Fixed Key", "../results/training_curves/fixed_key_curve.png")
    
    print("Evaluating Model...")
    loss, accuracy = model.evaluate(X_test, y_test, verbose=0)
    print(f"Test Accuracy: {accuracy*100:.2f}%")
    
    # Since we are attacking the Hamming Weight directly and trained on it,
    # the accuracy directly reflects our ability to recover the leakage.
    # In a full key recovery, we would use the model predictions to rank the key guesses.
    
    print("Attack complete!")

if __name__ == "__main__":
    main()
