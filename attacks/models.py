import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Conv1D, MaxPooling1D, Flatten, Dense, BatchNormalization, Activation
from tensorflow.keras.optimizers import Adam

def create_cnn_model(input_length, num_classes=9): # 9 classes for HW (0 to 8)
    model = Sequential([
        # Block 1
        Conv1D(filters=8, kernel_size=10, strides=1, padding='same', input_shape=(input_length, 1)),
        BatchNormalization(),
        Activation('relu'),
        MaxPooling1D(pool_size=2, strides=2),
        
        # Block 2
        Conv1D(filters=16, kernel_size=10, strides=1, padding='same'),
        BatchNormalization(),
        Activation('relu'),
        MaxPooling1D(pool_size=2, strides=2),
        
        # FC layers
        Flatten(),
        Dense(128),
        BatchNormalization(),
        Activation('relu'),
        Dense(num_classes, activation='softmax')
    ])
    
    optimizer = Adam(learning_rate=0.001)
    model.compile(loss='sparse_categorical_crossentropy', optimizer=optimizer, metrics=['accuracy'])
    
    return model
