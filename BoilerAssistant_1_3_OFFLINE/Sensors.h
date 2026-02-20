/*
 * ============================================================
 *  Boiler Assistant – Sensor Interface Module (v1.3)
 *  ------------------------------------------------------------
 *  File: Sensors.h
 *  Author: The Architect Collective
 *  License: CC BY-NC-SA 4.0
 *
 *  Description:
 *      Public interface for all sensor‑related functions used by
 *      the Boiler Assistant firmware. This module provides:
 *
 *          • Initialization of sensor hardware
 *          • Exhaust temperature acquisition (MAX31855)
 *          • Smoothed exhaust temperature filtering
 *          • Environmental sensor stubs (BME280)
 *          • Water probe stubs (DS18B20)
 *
 *      Notes:
 *          • MAX31855 is read via SPI (UNO R4 fixed SPI pins).
 *          • Only exhaust temperature is used in v1.3 logic.
 *          • Environmental and water sensors are placeholders
 *            for future expansion in v1.4.
 *
 *      This header is documentation-only and has ZERO effect on
 *      compiled code, logic, timing, or behavior.
 * ============================================================
 */

#pragma once
#include <Arduino.h>

// ============================================================
//  Initialization
// ============================================================
void sensors_init();

// ============================================================
//  Exhaust Temperature (Thermocouple)
// ============================================================
double exhaust_readF_cached();
double smoothExhaustF(double rawF);

// MAX31855 direct read (°F)
double readMAX31855_F();

// ============================================================
//  Environmental Sensor (BME280) – Stub
// ============================================================
float env_readTempF();
float env_readHumidity();
float env_readPressure();

// ============================================================
//  Water Temperature Probes (DS18B20) – Stub
// ============================================================
void sensors_readWaterTemps();
