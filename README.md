# Environmental Monitoring Station (Youth Climate Action Fund Evanston)
## Built by: **Adan Ciprian and Martin Suarez**

>Open source station code, documentation, BOM, and setup instructions.

Urban heat islands disproportionately affect vulnerable communities, where limited green space and high pavement concentrations can increase temperatures, worsen air quality, and raise energy costs. While existing climate data such as satellite surface temperature and canopy coverage can be useful for identifying areas with higher risks, ground-level environmental monitoring provides a clearer, higher-resolution picture of what residents experience in their everyday lives. Funded through the Youth Climate Action Fund (YCAF), our project aims to identify and better understand microclimate differences and unequal exposure to climate risks across Evanston by establishing a network of low-cost environmental monitoring stations, prioritizing neighborhoods with higher predicted climate risks while including areas with ample canopy coverage for comparison purposes. These stations will collect measurements of air temperature, surface temperature, air pollutants, and humidity, allowing us to examine how environmental conditions vary across Evanston. This project will culminate in a publicly accessible report with environmental maps and targeted recommendations to support public health, city planning, and future tree canopy expansion efforts. Evanston has already identified preserving and expanding its urban tree canopy as an important component of its climate action and resilience goals. Our project will support these efforts by providing more localized environmental data to inform future decisions, helping city officials and community members better understand where environmental challenges may be most pronounced and where climate action could be most beneficial. By combining community-focused monitoring with direct recommendations, we hope to provide a resource that supports a healthier and more climate-resilient Evanston, while contributing to the long-term vision outlined in Envision Evanston 2045.

### Prototype BOM:

| Component | Link | Price | Notes |
|---|---|---|---|---|
| Adafruit ESP32 S3 Feather | https://www.adafruit.com/product/5323 | $17.5 | One of the best microcontrollers on the market with the added benefit of seemless integration around the Adafruit feather ecosystem. |
| Adafruit Featherwing Datalogger | https://www.adafruit.com/product/2922 | $8.95 | The datalogger featherwing can be easily attached to the esp32 s3 microcontroller and leverages an RTC (Real Time Clock) with independent SD storage, ensuring secure data and accurate timestamps. |
| CR1220 Lithium Ion Battery for RTC | https://www.amazon.com/Cotchear-Battery-Lithium-Batteries-Electronic/dp/B07XQBL1M6 | 3.99 | Any CR1220 works to power the datalogger rtc. |
| 16 GB MicroSD Card| https://www.amazon.com/SanDisk-Ultra-SDSQUNS-016G-GN3MN-UHS-I-microSDHC/dp/B074B4P7KD | 17.13 | Excellent SD Card for data storage. Rated for weather resistance and outdoor deployment. |
| BME 680 Temp/Humidity/Pressure/VOC Sensor | https://www.adafruit.com/product/3660 | $18.95 | The BME 680 is one of the foremost environmental sensors which provides spot-on measurements even when compared to state-of-the-art systems. |
| PMS 5003 Air Quality Sensor | https://www.adafruit.com/product/3686 | $39.99 | Excellent particular matter sensor commonly used in city-wide environmental monitoring stations with special accuracy for PM2.5, one of the most notorious pollutants. |
| MLX90614 Infrared Surface Temperature Sensor | https://www.amazon.com/HiLetgo-MLX90614ESF-Non-contact-Infrared-Temperature/dp/B071VF2RWM | $15.99 | Excellent for higher resolution temperature readings to demonstrate the adverse effects of high pavement and asphalt concentrations. |
| Magnetic Contact Switch | https://www.adafruit.com/product/375 | $3.95 | Used to trigger transfer of data from the SD card to another device. |
| Lithium Ion Cylindrical Battery 2200mAh | https://www.adafruit.com/product/1781 | $9.95 | Sturdy and puncture-resistant rechargeable battery that can provide up to a week of energy in conditions without sunlight. |
|Solar Panel (6V 5W) | https://www.amazon.com/Solar-Panel-6V-Performance156-Monocrystalline/dp/B07PNTRSNY | $24.99 |  Solar panel to provide electricity for prolonged deployment. |
| bq25185 Solar to Battery Converter and 5V Booster |  https://www.adafruit.com/product/6106 | $8.95 | Regulator between solar panel and battery with voltage booster for the pms5003's power |
| Solar Panel Pole Mount | https://www.amazon.com/TPSM-5-10-SP-Accomodates-Included-U-Bolts-Survivability/dp/B0C1DDMJQM | $16.04 | Mount for solar panel onto pole |
| Weather Resistant Electronics Enclosure | https://www.amazon.com/LeMotech-Waterproof-Electrical-Dustproof-Enclosure/dp/B0B5QGM83Y?th=1 | $6.99 | Keeps circuitry safe and cool. Double check size. |
| Pole Mount Bracket | https://www.amazon.com/Lighfast-Universal-Mounting-Stainless-Vertical/dp/B0BVT4J3FF | $9.99 | To aid with the mounting of other components. |
| Sensor Weather Shield | https://www.amazon.com/Crosse-Technology-925-1418-Sensor-Protection/dp/B00VSXENM4 | $19.95 | Standard industry-grade enclosure to protect sensors from negative weather while still facilitating reliable measurements. |

