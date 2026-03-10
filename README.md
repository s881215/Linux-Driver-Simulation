# 嵌入式 Linux 驅動模擬：虛擬觸控與伺服馬達控制系統

這個專案是一個整合性的嵌入式 Linux 驅動程式實作，旨在模擬觸控 IC 產品的底層開發架構。透過實體按鈕觸發硬體中斷，同時驅動伺服馬達並向系統回報虛擬觸控座標，展現了從硬體中斷、核心驅動到用戶空間應用的完整數據鏈路。

## 🚀 技術層面

* **中斷處理機制 (Interrupt Management)**：使用 `request_threaded_irq` 實作 Top/Bottom Half，確保馬達控制等耗時任務不會阻塞系統中斷回應。
* **Linux Input Subsystem 模擬**：將驅動註冊為輸入設備，模擬觸控點擊 (`BTN_TOUCH`) 與絕對座標 (`ABS_X`, `ABS_Y`) 回報。
* **PWM 精確控制**：利用核心 PWM 子系統驅動伺服馬達，實現自動化角度切換 (0° ↔ 90°)。
* **Sysfs 交互介面**：提供 `/sys/kernel/my_servo/servo_angle` 檔案節點，支援用戶空間腳本直接操作。
* **Delta Update 解決方案**：在驅動層實作 `toggle` 座標偏移機制，確保 Linux Input 核心不會因為數值相同而過濾掉重複的點擊事件。

## 🛠 硬體配置

* **平台**：Raspberry Pi 4B / 5
* **輸入**：彈跳按鈕接至 GPIO 17 (Pin 11)
* **輸出**：SG90 伺服馬達訊號線接至 PWM 1 (GPIO 19 / Pin 35)

## 📂 檔案清單

* `motor_driver.c`：核心驅動程式，處理 GPIO、PWM、Threaded IRQ 與 Input Device 註冊。
* `monitor.c`：User Space 監控工具，讀取並解析 `/dev/input/eventX` 的二進制事件數據。
* `demo.sh`：自動化測試腳本，處理模組編譯後的掛載、節點檢查與初步測試。

## 📝 快速開始

### 1. 系統環境準備
確保 `/boot/firmware/config.txt` 已啟用 PWM：
```text
[all]
dtoverlay=pwm-2chan
