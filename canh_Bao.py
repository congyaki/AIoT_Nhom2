import pandas as pd
import dweepy
from threading import Event, Thread, Lock
import time
import pyodbc
from keras.models import load_model
import numpy as np
from datetime import datetime

class DweetPredictor:
    def __init__(self, model_path, thing_name):
        self.model = load_model(model_path)
        self.thing_name = thing_name
        self.labels = ['Low', 'Normal', 'High']
        self.conn = pyodbc.connect('Driver={SQL Server};'
                                   'Server=DESKTOP-9DMP1J1\THANGNE;'
                                   'Database=IOT;'
                                   'Trusted_Connection=yes;')
        self.cursor = self.conn.cursor()
        self.last_time = ""
        self.event = Event()
        self.lock = Lock()  # Add a lock for thread safety

    def preprocess_data(self, data_dict):
        longdate = data_dict['created']
        date = longdate[:10]
        timeStamp = longdate[11:19]
        t = data_dict['content']["t"]
        h = data_dict['content']["h"]
        a = data_dict['content']["a"]
        s = data_dict['content']["s"]
        g = data_dict['content']["g"]

        input_features = pd.DataFrame({ 'h': [h],'t': [t],'g': [g], 'a': [a], 's': [s] })
        input_features = input_features.values.astype(float)

        return input_features, longdate

    def run_prediction(self):
        while not self.event.is_set():
            try:
                url = dweepy.get_latest_dweet_for(self.thing_name)
                data_dict = url[0]
                
                input_features, longdate = self.preprocess_data(data_dict)

                # Make predictions using the trained model
                predictions = self.model.predict(input_features)

                # Assuming you want to get the class with the highest probability as the predicted class
                predicted_class = np.argmax(predictions, axis=1)[0]

                if self.last_time != longdate:
                    predicted_label = self.labels[predicted_class]

                    # Insert the prediction result into the database
                    with self.lock:  # Use the lock to ensure thread safety
                        self.cursor.execute(
                            f"INSERT INTO sMart(Humidity, Temperature, Gas, Noise, Light, Time, CanhBao) VALUES({input_features[0][0]}, {input_features[0][1]}, {input_features[0][2]}, {input_features[0][3]}, {input_features[0][4]}, '{longdate}', '{predicted_label}')"
                        )
                        self.conn.commit()
                        self.last_time = longdate

                        print("Input Features:", input_features)
                        print("Predictions:", predictions)
                        print("Predicted Class Index:", predicted_class)
                        print("Predicted Label:", predicted_label)

            except Exception as e:
                print("Error:", e)
                time.sleep(2)

    def stop(self):
        self.event.set()

def main():
    model_path = 'D:/IOT/trained_model.h5'
    thing_name = "nhom6"

    dweet_predictor = DweetPredictor(model_path, thing_name)
    
    # Create and start the data retrieval thread
    getdata_thread = Thread(target=dweet_predictor.run_prediction)
    getdata_thread.start()

    try:
        # Wait for a period of time (e.g., 5 seconds)
        time.sleep(5)
    except KeyboardInterrupt:
        print("Main thread terminated.")
        dweet_predictor.stop()

if __name__ == '__main__':
    main()