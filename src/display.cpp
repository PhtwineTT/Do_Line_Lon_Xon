#include "display.h"
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Khởi tạo
void setupDisplay() {
    Wire.begin(OLED_SDA, OLED_SCL, 400000);
    if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(10, 25);
        display.println("DO LINE LON XON");
        display.display();
        delay(500);
    }
}

// Hiện trạng thái theo map
void updateOledDashboard(int selectedMap, MapProfile profiles[], float currentError, int leftCount, int rightCount) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print("> ");
    display.println(profiles[selectedMap].name);
    display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    display.setCursor(0, 14);
    display.printf("Kp:%.3f Kd:%.2f V:%d",
                   profiles[selectedMap].kp,
                   profiles[selectedMap].kd,
                   profiles[selectedMap].maxBaseSpeed);

    display.setCursor(0, 26);
    display.printf("Err: %6.0f | L:%d R:%d", currentError, leftCount, rightCount);
    for (int i = 0; i < NUM_SENSORS; i++) {
        int barHeight = map(normSensor[i], 0, 1000, 1, 24);
        int xPos = i * 8; 
        display.drawRect(xPos, 39, 6, 24, SSD1306_WHITE);
        display.fillRect(xPos, 63 - barHeight, 6, barHeight, SSD1306_WHITE);
    }

    display.display();
}

// Hiển thị trạng thái hiệu chuẩn (Tối ưu tốc độ quét mẫu)
void runAutoCalibration() {
    resetCalibData();
    unsigned long start = millis();
    unsigned long lastDraw = 0;

    while (millis() - start < 4000) {
        // Đọc liên tục ở tốc độ tối đa để không bỏ sót đỉnh đen/trắng
        readAllRawSensors();
        for (int i = 0; i < NUM_SENSORS; i++) {
            if (rawSensor[i] < minVal[i]) minVal[i] = rawSensor[i];
            if (rawSensor[i] > maxVal[i]) maxVal[i] = rawSensor[i];
        }

        // Chỉ vẽ lại màn hình OLED mỗi 100ms để không làm chậm quá trình lấy mẫu
        if (millis() - lastDraw >= 100) {
            lastDraw = millis();
            int progress = map(millis() - start, 0, 4000, 0, 120);
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(18, 12);
            display.println("CALIBRATING...");
            display.setCursor(8, 28);
            display.println("Quet dau xe qua line");
            display.drawRect(4, 46, 120, 10, SSD1306_WHITE);
            display.fillRect(4, 46, progress, 10, SSD1306_WHITE);
            display.display();
        }
    }
}