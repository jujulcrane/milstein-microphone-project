/*
 ESP32 Connect to Wi-Fi
 http:://www.electronicwings.com
*/ 


/*
 ESP32 Connect to Wi-Fi and Stream Audio to Server
 http:://www.electronicwings.com
*/ 

#include <WiFi.h>
#include <HTTPClient.h>
#include <driver/i2s.h>

#define I2S_WS 25
#define I2S_SD 33
#define I2S_SCK 32

// Use I2S Processor 0
#define I2S_PORT I2S_NUM_0
 
// Define input buffer length for I2S reads
#define bufferLen 64
int16_t sBuffer[bufferLen];

const int SAMPLE_RATE = 16000;
// Define chunk buffer size (adjust based on your needs)
#define CHUNK_SIZE SAMPLE_RATE * 1  // 2 second chunks
uint8_t chunkBuffer[CHUNK_SIZE];
size_t chunkIndex = 0;

// Replace with your network credentials
const char* ssid = "DIGITALMAGIC-2.4G"; 
const char* password = "DIGITALMAGIC2025!";

// Server configuration
const char* transcriptionURL = "http://192.168.8.228:5000/transcription";
const char* serverURL = "http://192.168.8.228:5000/audio";

HTTPClient http;
WiFiClient client;

//////////////// ESP32 I2S Setup /////////////////
void i2s_install() {
  // Set up I2S Processor configuration
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,  // Changed to 16kHz for better Whisper compatibility,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = bufferLen,
    .use_apll = false
  };
 
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
}

void i2s_setpin() {
  // Set I2S pin configuration
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };
 
  i2s_set_pin(I2S_PORT, &pin_config);
}

void initWiFi() {
 WiFi.mode(WIFI_STA);    //Set Wi-Fi Mode as station
 WiFi.begin(ssid, password);   
 
 Serial.println("Connecting to WiFi ..");
 while (WiFi.status() != WL_CONNECTED) {
   Serial.print('.');
   delay(1000);
 }
 
 Serial.println(WiFi.localIP());
 Serial.print("RRSI: ");
 Serial.println(WiFi.RSSI());
}

void sendChunkToServer() {
  if (WiFi.status() == WL_CONNECTED) {
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/octet-stream");
    //http.addHeader("Content-Length", String(chunkIndex));
    
    Serial.print("Sending chunk to server (");
    Serial.print(chunkIndex);
    Serial.println(" bytes)...");
    
    int httpResponseCode = http.POST((uint8_t*)chunkBuffer, CHUNK_SIZE * sizeof(int16_t));
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("HTTP Response: ");
      Serial.println(httpResponseCode);
      if (response.length() > 0) {
        Serial.print("Server response: ");
        Serial.println(response);
      }
    } else {
      Serial.print("HTTP Error: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  } else {
    Serial.println("WiFi Disconnected - cannot send chunk");
  }
  
  // Reset chunk buffer
  chunkIndex = 0;
}

void setup() {
 Serial.begin(115200);
 initWiFi();
  Serial.println("WiFi setup complete");
 
  delay(1000);
 
  // Set up I2S
  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);
  Serial.println("Microphone setup complete");
 
 
  delay(500);

  // Initialize chunk buffer
  chunkIndex = 0;
  
  Serial.println("Entering main loop - streaming audio to server...");
}

void loop() {
 // Read audio sample
  size_t bytesIn = 0;
  esp_err_t result = i2s_read(I2S_PORT, &sBuffer, bufferLen * sizeof(int16_t), &bytesIn, portMAX_DELAY);
  //int sample_read = bytesIn / sizeof(int16_t);
 
  if (result == ESP_OK && bytesIn > 0) {
    // Fill chunk buffer with audio data
    size_t bytesToCopy = min(bytesIn, CHUNK_SIZE - chunkIndex);
    
    if (bytesToCopy > 0) {
      memcpy(chunkBuffer + chunkIndex, sBuffer, bytesToCopy);
      chunkIndex += bytesToCopy;
      
      // Check if chunk buffer is full
      if (chunkIndex >= CHUNK_SIZE) {
        // Send HTTP POST to server
        sendChunkToServer();
        chunkIndex = 0;
        // chunkIndex is reset to 0 in sendChunkToServer()
      }
    }
  }
  
  // Small delay to prevent overwhelming the system
  delayMicroseconds(100);
}
