/*
 * BB-R2 WiFi Configuration Header
 * Bakken Museum Workshop - December 2025
 * 
 * INSTRUCTIONS:
 * 1. Set DROID_NUMBER for each droid (0-16)
 * 2. Set BOARD_TYPE for your ESP32 variant
 * 3. Save and upload
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// DROID IDENTITY
// ============================================================================
// Change this number for each droid: 0 = demo unit, 1-16 = workshop droids
// This auto-generates the WiFi SSID and password

#define DROID_NUMBER 1

// ============================================================================
// BOARD SELECTION
// ============================================================================
// Uncomment ONE of these lines based on your ESP32 board:

// #define BOARD_XIAO_ESP32C3      // Seeed Xiao ESP32C3 (original dev board)
#define BOARD_ESP32_DEVKIT      // Generic ESP32-WROOM-32 DevKit / NodeMCU

// ============================================================================
// FEATURE TOGGLES
// ============================================================================

#define ENABLE_SOUND        true   // Set false to disable DFPlayer code
#define ENABLE_BATTERY_MON  true   // Set false to disable battery monitoring
#define ENABLE_LED          true   // Set false to disable LED control

// ============================================================================
// SERVO TUNING
// ============================================================================

#define SERVO_SPEED         5      // 1 = slow/smooth, 10 = fast/snappy (default: 5)
#define SERVO_MIN           40     // Minimum servo angle
#define SERVO_MAX           150    // Maximum servo angle
#define SERVO_CENTER        90     // Neutral/stop position

// Movement speed (distance from center)
#define DRIVE_SPEED         40     // Forward/back: center ± this value
#define TURN_SPEED          40     // Turning: one wheel forward, one back
#define DOME_SPEED          50     // Dome rotation speed

// ============================================================================
// PIN ASSIGNMENTS - AUTO-CONFIGURED BASED ON BOARD_TYPE
// ============================================================================

#ifdef BOARD_XIAO_ESP32C3
  // Seeed Xiao ESP32C3 pinout
  #define SERVO_LEFT_PIN    2
  #define SERVO_RIGHT_PIN   3
  #define SERVO_DOME_PIN    4
  #define LED_PIN           5
  #define DFPLAYER_RX       20
  #define DFPLAYER_TX       21
  #define BATTERY_PIN       A0
  
#elif defined(BOARD_ESP32_DEVKIT)
  // ESP32-WROOM-32 DevKit / NodeMCU pinout
  // Avoiding: GPIO 0 (boot), 1 (TX0), 3 (RX0), 6-11 (flash SPI)
  #define SERVO_LEFT_PIN    13
  #define SERVO_RIGHT_PIN   12
  #define SERVO_DOME_PIN    14
  #define LED_PIN           2      // Built-in LED on most DevKits
  #define DFPLAYER_RX       16
  #define DFPLAYER_TX       17
  #define BATTERY_PIN       34     // ADC1 channel (GPIO 34 is input-only, safe for ADC)
  
#else
  #error "No board type defined! Uncomment BOARD_XIAO_ESP32C3 or BOARD_ESP32_DEVKIT in config.h"
#endif

// ============================================================================
// WIFI CREDENTIALS - AUTO-GENERATED FROM DROID_NUMBER
// ============================================================================

// Helper macros for string concatenation
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// Format: R2-BK00, R2-BK01, ... R2-BK16
#if DROID_NUMBER < 10
  #define WIFI_SSID "R2-BK0" TOSTRING(DROID_NUMBER)
  #define WIFI_PASS "droidBK0" TOSTRING(DROID_NUMBER)
#else
  #define WIFI_SSID "R2-BK" TOSTRING(DROID_NUMBER)
  #define WIFI_PASS "droidBK" TOSTRING(DROID_NUMBER)
#endif

// ============================================================================
// BATTERY MONITORING
// ============================================================================

#define BATTERY_R1          10000.0   // Voltage divider R1 (ohms)
#define BATTERY_R2          10000.0   // Voltage divider R2 (ohms)
#define ADC_RESOLUTION      4095      // 12-bit ADC
#define BATTERY_MIN_MV      4400      // 4x AA minimum voltage (mV)
#define BATTERY_MAX_MV      6000      // 4x AA maximum voltage (mV)

// ============================================================================
// DEBUG OUTPUT
// ============================================================================

#define DEBUG_SERIAL        true      // Enable Serial.print debug messages
#define SERIAL_BAUD         115200    // Serial monitor baud rate

#endif // CONFIG_H
