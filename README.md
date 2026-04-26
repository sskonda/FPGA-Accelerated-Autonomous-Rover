# FPGA-Accelerated-Autonomous-Rover

Autonomous FPGA-based mobile robot that follows lanes, detects obstacles, monitors system telemetry, locates a charging dock, and demonstrates wireless charging behavior using a Zynq-based FPGA SoC.

This project is also referred to as **ERN**: **FPGA-Based Electronic Rechargeable Navigator**.

## Table of Contents

- [Project Purpose](#project-purpose)
- [Problem Being Addressed](#problem-being-addressed)
- [Project Overview](#project-overview)
- [Why an SoC Instead of a Microcontroller, Raspberry Pi, or Jetson?](#why-an-soc-instead-of-a-microcontroller-raspberry-pi-or-jetson)
  - [Compared to a Microcontroller](#compared-to-a-microcontroller)
  - [Compared to a Raspberry Pi](#compared-to-a-raspberry-pi)
  - [Compared to a Jetson](#compared-to-a-jetson)
  - [Why the SoC Approach Fits This Rover Best](#why-the-soc-approach-fits-this-rover-best)
  - [Platform Comparison](#platform-comparison)
  - [The Main Engineering Point](#the-main-engineering-point)
  - [Why This Makes the Project More Impressive](#why-this-makes-the-project-more-impressive)
- [Main System Capabilities](#main-system-capabilities)
  - [Implemented or Integrated](#implemented-or-integrated)
  - [Planned or Demonstrated Behavior](#planned-or-demonstrated-behavior)
- [System Architecture](#system-architecture)
- [Hardware/Software Partitioning](#hardwaresoftware-partitioning)
- [Hardware Platform](#hardware-platform)
  - [Main Compute Platform](#main-compute-platform)
  - [Robot Chassis and Drive System](#robot-chassis-and-drive-system)
  - [Sensors and Peripherals](#sensors-and-peripherals)
- [Vivado FPGA Design](#vivado-fpga-design)
- [Embedded Software Architecture](#embedded-software-architecture)
  - [FreeRTOS Task Model](#freertos-task-model)
- [Camera and Video Pipeline](#camera-and-video-pipeline)
- [Line-Following Algorithm](#line-following-algorithm)
  - [HDMI Debug Overlay](#hdmi-debug-overlay)
- [Custom Motor-Control IP](#custom-motor-control-ip)
  - [Register Map](#register-map)
  - [Control Bit Layout](#control-bit-layout)
  - [Differential Drive Usage](#differential-drive-usage)
- [Sensor Integration](#sensor-integration)
  - [BNO055 IMU](#bno055-imu)
  - [INA260 Power Monitor](#ina260-power-monitor)
  - [MAXSONAR Obstacle Sensor](#maxsonar-obstacle-sensor)
- [Obstacle Detection and Safe Stop](#obstacle-detection-and-safe-stop)
- [Audio Feedback System](#audio-feedback-system)
- [Power Monitoring and Wireless Charging](#power-monitoring-and-wireless-charging)
- [Repository Structure](#repository-structure)
- [Important Source Files](#important-source-files)
- [Build Environment](#build-environment)
- [Bring-Up and Test Strategy](#bring-up-and-test-strategy)
  - [1. Video Only](#1-video-only)
  - [2. Motor IP Only](#2-motor-ip-only)
  - [3. BNO055 Only](#3-bno055-only)
  - [4. INA260 Only](#4-ina260-only)
  - [5. Sonar Only](#5-sonar-only)
  - [6. Audio Only](#6-audio-only)
  - [7. FreeRTOS Integration](#7-freertos-integration)
  - [8. Full Robot Test](#8-full-robot-test)
- [Typical UART Output](#typical-uart-output)
- [Engineering Challenges and Fixes](#engineering-challenges-and-fixes)
  - [Camera Pipeline Bring-Up](#camera-pipeline-bring-up)
  - [Vitis and xparameters.h Compatibility](#vitis-and-xparametersh-compatibility)
  - [FreeRTOS and Interrupt Conflicts](#freertos-and-interrupt-conflicts)
  - [Sensor Initialization Blocking](#sensor-initialization-blocking)
  - [UART Logging From Multiple Tasks](#uart-logging-from-multiple-tasks)
  - [Audio Integration](#audio-integration)
  - [Motor Control and Duty Tuning](#motor-control-and-duty-tuning)
- [Current Status](#current-status)
- [Known Limitations](#known-limitations)
- [Future Work](#future-work)
- [Skills Demonstrated](#skills-demonstrated)
- [Team](#team)
- [Project Summary](#project-summary)

## Project Purpose

The purpose of this project is to design and build a small-scale autonomous guided vehicle that demonstrates the value of **FPGA-based real-time control** in an embedded robotics system.

The rover is designed to behave like a simplified autonomous mobile robot used in warehouse, factory, or material-handling environments. It must detect a visual path on the ground, follow that path, stop safely when an obstacle is present, monitor its own electrical state, and eventually navigate toward a charging station when battery level is low.

The important part of this project is not only that the robot can move. The main purpose is to demonstrate a complete **hardware/software co-design** approach:

- Real-time camera and video infrastructure is handled using FPGA-oriented hardware blocks.
- Motor timing is generated deterministically using custom programmable logic.
- High-level behavior, telemetry, task scheduling, and control decisions run on the ARM processor.
- FreeRTOS coordinates multiple concurrent robot subsystems.
- Power, sensing, actuation, perception, audio, and charging are integrated into one platform.

This makes the project more than a basic robotics demo. It is a complete embedded SoC system that combines computer vision, FPGA logic, real-time software, custom hardware IP, sensor integration, and power electronics.

## Problem Being Addressed

Repetitive material-handling tasks in warehouses and factories are commonly automated using autonomous guided vehicles or autonomous mobile robots. These robots transport materials along predefined routes, avoid obstacles, and return to charging stations when power is low.

Many small autonomous robots are built around software-centric platforms such as Raspberry Pi boards, Jetson modules, or microcontrollers. Those platforms are flexible and easy to prototype with, but they often push every major subsystem through software. Camera processing, motor control, telemetry, obstacle detection, audio, and decision logic all compete for CPU time, memory bandwidth, and operating-system scheduling.

That creates several problems:

- **Higher latency** for time-sensitive camera and control tasks
- **Less predictable timing** when multiple software tasks run at once
- **Higher power usage** compared to dedicated hardware logic
- **More CPU contention** between vision, telemetry, control, and I/O
- **Less deterministic motor control** when PWM generation depends on software timing
- **Reduced scalability** when adding more sensors, actuators, or perception features

This project explores a different approach: use an FPGA SoC to separate the system into hardware tasks and software tasks. The FPGA fabric handles deterministic and timing-sensitive work, while the ARM processor handles high-level system behavior.

## Project Overview

The FPGA-Accelerated Autonomous Rover is built around the **Digilent Zybo Z7-20**, which contains a **Xilinx Zynq-7000 SoC**. The Zynq device combines an ARM Cortex-A9 processing system with FPGA programmable logic on the same chip.

The rover uses:

- A **PCam 5C camera** for visual lane detection
- A **Zybo Z7-20 FPGA SoC** for video, control, and peripheral integration
- A **custom AXI4-Lite motor-control IP** for deterministic PWM generation
- A **BNO055 IMU** for motion telemetry and navigation experiments
- An **INA260 power monitor** for voltage/current/power telemetry
- A **MAXSONAR ultrasonic range sensor** for obstacle detection
- The **Zybo onboard SSM2603 audio codec** for audio feedback
- A **wireless charging dock** to demonstrate autonomous recharging behavior

The robot's intended behavior is:

1. Boot the FPGA and embedded software.
2. Initialize the camera, video pipeline, motor IP, sensors, sonar, telemetry, and optional audio.
3. Display live camera output over HDMI.
4. Detect a tape-based lane in the camera frame.
5. Compute the lane center relative to the image center.
6. Adjust left and right motor duty cycles to keep the lane centered.
7. Stop or brake if an obstacle is detected.
8. Monitor battery voltage/current.
9. Navigate toward a charging location or dock when required.
10. Demonstrate wireless charging by measuring a voltage/current increase at the charging location.

## Why an SoC Instead of a Microcontroller, Raspberry Pi, or Jetson?

A major purpose of this project was to demonstrate why a **System-on-Chip FPGA platform** is valuable for real-time embedded robotics. The rover could have been built with a microcontroller, Raspberry Pi, Jetson Nano, or another software-centric embedded computer, but each of those platforms would force a different set of compromises.

The Zybo Z7-20 is built around a Zynq-7000 SoC, which combines two important computing resources on the same chip:

1. **Processing System (PS):** ARM Cortex-A9 processor cores for software, control logic, system initialization, telemetry, and FreeRTOS task scheduling.
2. **Programmable Logic (PL):** FPGA fabric for deterministic hardware acceleration, custom peripherals, video movement, PWM generation, and timing-sensitive interfaces.

That combination is the central reason this project uses an SoC. The robot is not just running code on a processor. It is dividing the system into hardware and software responsibilities.

### Compared to a Microcontroller

A microcontroller would be good for basic motor control, sensor polling, and simple real-time logic. If the project only needed to read a few sensors and drive a few PWM signals, a microcontroller would be simpler, cheaper, and lower power.

However, this rover requires more than simple control. It needs to handle camera input, video frame movement, line detection, motor control, telemetry, obstacle sensing, audio output, and power monitoring at the same time.

A microcontroller would struggle with this system for several reasons:

- It does not have enough memory bandwidth for high-resolution video.
- It usually cannot handle 1920x1080 camera frame buffers directly.
- It has limited RAM compared to the size of full video frames.
- It cannot easily implement custom high-throughput hardware pipelines.
- It would need to time-share the CPU between vision, control, telemetry, and communication.
- Adding more hardware interfaces quickly becomes limited by fixed peripherals.
- Complex video or image-processing tasks would either be too slow or require external hardware.

For example, the rover's camera pipeline uses 1920x1080 RGB frames. A single RGB888 frame is approximately:

```text
1920 * 1080 * 3 bytes = 6,220,800 bytes ≈ 5.93 MiB
```

With triple buffering:

```text
3 * 6,220,800 bytes = 18,662,400 bytes ≈ 17.8 MiB
```

That alone is far beyond what many microcontrollers can comfortably store or process. A microcontroller can control a robot, but it is not the right platform for a robot that combines live video, frame buffering, real-time perception, multiple sensors, and custom hardware control.

In this project, the FPGA SoC solves that problem by allowing the video pipeline and custom motor-control hardware to exist outside the normal CPU execution path.

### Compared to a Raspberry Pi

A Raspberry Pi would be easier to develop on because it can run Linux, OpenCV, Python, ROS, and high-level software tools. It is a strong choice for quick robotics prototyping, networking, and general software development.

However, a Raspberry Pi is still mostly a software-centric platform. The CPU and GPU are powerful, but the system is not designed around user-defined hardware acceleration. For this rover, that creates several disadvantages:

- Camera processing, control logic, telemetry, and I/O compete for CPU time.
- Linux is not naturally deterministic for hard real-time motor-control behavior.
- PWM and timing-sensitive outputs may depend on software, kernel drivers, or external controllers.
- Custom hardware peripherals cannot be added directly into the processor fabric.
- Real-time control can be affected by OS scheduling, background processes, and driver behavior.
- The system does not naturally demonstrate hardware/software co-design.

A Raspberry Pi would make the robot easier to program, but it would also make the project less impressive from an SoC design perspective. The goal of this project was not just to make a robot follow a line. The goal was to build a system where the hardware architecture itself improves timing, modularity, and real-time behavior.

On the Zybo Z7-20, the FPGA fabric can generate motor PWM deterministically while the ARM processor runs FreeRTOS tasks. The video pipeline can continue moving frames through VDMA and HDMI while software reads only the information it needs for control. This separation is much closer to how real embedded vision and industrial control systems are designed.

### Compared to a Jetson

A Jetson Nano or Jetson Orin Nano would be much stronger than a Raspberry Pi for AI inference, GPU acceleration, neural networks, and high-level computer vision. If the main goal were deep-learning-based object detection or neural-network inference, a Jetson would be a very reasonable platform.

However, a Jetson is still primarily a CPU/GPU embedded computer. It accelerates computation using a GPU, but it does not provide the same kind of custom deterministic digital hardware that an FPGA gives.

For this project, the Jetson approach has several drawbacks:

- It is optimized for AI/GPU workloads, not custom motor-control hardware.
- It still relies heavily on software stacks, drivers, CUDA, Linux, and GPU scheduling.
- Deterministic low-level I/O may still require external microcontrollers or motor controllers.
- It consumes more power than is necessary for simple lane following.
- It does not demonstrate custom AXI peripheral design.
- It does not allow the same level of hardware-defined timing and signal control.
- It would make the project more about AI software than SoC hardware/software co-design.

The rover does not need a large neural network to follow a high-contrast tape lane. The more interesting engineering problem is how to move camera data, generate motor signals, read sensors, provide audio feedback, and run real-time tasks in a clean embedded architecture. That is exactly where a Zynq SoC is valuable.

### Why the SoC Approach Fits This Rover Best

The SoC approach is the best fit because this rover has both **high-level software needs** and **low-level deterministic hardware needs**.

The ARM processor handles:

- FreeRTOS task scheduling
- Sensor initialization
- Telemetry output
- Lane-following decision logic
- Obstacle state handling
- Audio state triggers
- Charging state logic
- Debug and runtime control

The FPGA fabric handles or supports:

- Camera data movement
- HDMI video output
- AXI VDMA frame buffering
- Custom motor PWM generation
- AXI memory-mapped peripherals
- Audio hardware interfaces
- Timing-sensitive signal generation
- Future image-processing acceleration

This division is the main architectural strength of the project. Instead of forcing every task through one processor, the system uses the right computing resource for each job.

### Platform Comparison

| Platform | Strengths | Weaknesses for This Project | Why the SoC Is Better Here |
|---|---|---|---|
| Microcontroller | Simple, low power, deterministic, good for basic control | Limited RAM, limited video capability, limited processing throughput, fixed peripherals | The SoC can still do real-time control while also supporting video, DDR frame buffers, custom IP, and hardware acceleration |
| Raspberry Pi | Easy software development, Linux support, OpenCV/ROS support, strong community | Software-centric, less deterministic timing, limited custom hardware integration, CPU/GPU contention | The SoC provides programmable logic for deterministic PWM, hardware video movement, and custom AXI peripherals |
| Jetson | Strong GPU/AI performance, good for neural-network inference and advanced vision | Higher power, Linux/GPU stack complexity, less direct custom hardware control, overkill for tape lane following | The SoC is better for deterministic embedded vision/control without relying on a heavy AI/GPU pipeline |
| Zynq FPGA SoC | ARM processor plus FPGA fabric, custom hardware acceleration, deterministic I/O, AXI integration | More complex development flow, requires Vivado/Vitis knowledge, harder initial bring-up | Best match for demonstrating real hardware/software co-design in an embedded robotics system |

### The Main Engineering Point

The main engineering point of this project is that an autonomous robot is not just a software problem. It is a system-level architecture problem.

A software-only design asks:

```text
How fast can the CPU process everything?
```

This SoC design asks:

```text
Which parts should be hardware, which parts should be software, and how should they communicate?
```

That is the more important question for real embedded systems.

For this rover:

- Motor PWM belongs in hardware because timing should be deterministic.
- Video movement belongs in hardware because frame data is large and continuous.
- Sensor polling belongs in software because it is periodic and easier to debug.
- High-level behavior belongs in software because the robot state machine changes during development.
- Future lane preprocessing can move into hardware because pixel operations are parallel.
- Telemetry belongs in software because it is slow compared to the hardware data path.

This is why the project uses a Zynq SoC. It lets the rover behave like a true embedded hardware/software system instead of a general-purpose computer attached to motors.

### Why This Makes the Project More Impressive

Using an SoC makes the project more technically meaningful because it demonstrates skills that are not shown by a typical Raspberry Pi or Jetson robot.

This project demonstrates:

- Custom AXI peripheral design
- FPGA-based PWM generation
- Processor-to-FPGA memory-mapped control
- MIPI camera integration
- AXI VDMA video buffering
- HDMI video output
- FreeRTOS task scheduling on ARM
- Hardware/software partitioning
- Real-time embedded debugging
- Multi-peripheral system integration
- Sensor and actuator coordination
- Power and charging telemetry
- Full-system robotics architecture

A Raspberry Pi or Jetson implementation could show that the robot works. The Zynq SoC implementation shows how the robot is architected.

That distinction matters. The project is not just a line-following car. It is a demonstration of how an FPGA SoC can be used to build a real-time autonomous embedded system where perception, control, sensing, audio, telemetry, and power management are split across hardware and software in a deliberate way.

## Main System Capabilities

### Implemented or Integrated

- Zybo Z7-20 Zynq-7000 SoC platform
- PCam 5C camera input
- HDMI live video passthrough
- AXI VDMA frame buffering
- Triple-buffered 1920x1080 RGB video frames
- Custom 4-channel AXI4-Lite motor-control IP
- PWM control for four brushed DC motors
- Differential/tank-drive motion control
- FreeRTOS-based multitasking
- BNO055 IMU integration over PS I2C1
- INA260 power-monitoring integration over PS I2C1
- MAXSONAR range sensing through AXI UARTLite
- Obstacle stop behavior
- UART telemetry logging
- Audio feedback through SSM2603 codec, AXI Audio Formatter, and I2S transmitter
- Selectable subsystem test firmware
- Wireless charging dock firmware
- PCB files for charging, power, and peripheral boards

### Planned or Demonstrated Behavior

- Lane following using camera-derived line position
- HDMI debug overlay showing image center and detected lane center
- Safe stop when an obstacle is detected
- Battery voltage/current telemetry
- Charging dock identification
- Wireless charging voltage/current demonstration
- Autonomous state-machine behavior for navigation and charging

## System Architecture

```text
                         +------------------------------------------------+
                         |                Zybo Z7-20 Board                 |
                         |              Xilinx Zynq-7000 SoC               |
                         +------------------------------------------------+
                                             |
                  +--------------------------+--------------------------+
                  |                                                     |
                  v                                                     v
+--------------------------------------+        +--------------------------------------+
| Programmable Logic (FPGA / PL)       |        | Processing System (ARM / PS)        |
+--------------------------------------+        +--------------------------------------+
| MIPI CSI-2 / D-PHY camera pipeline   |        | FreeRTOS scheduler                  |
| AXI VDMA video frame movement        |        | Lane-following control logic        |
| HDMI video output infrastructure     |        | Sensor polling                      |
| Custom motor PWM controller          |        | Obstacle detection state handling   |
| AXI UARTLite for sonar               |        | Telemetry formatting and logging    |
| AXI Audio Formatter and I2S TX       |        | Audio trigger logic                 |
| AXI interconnects and peripherals    |        | High-level robot state machine      |
+--------------------------------------+        +--------------------------------------+
                  |                                                     |
                  +--------------------------+--------------------------+
                                             |
                                             v
                              +------------------------------+
                              |      Rover Hardware Body      |
                              +------------------------------+
                              | PCam 5C camera                |
                              | Four brushed DC motors        |
                              | TB67H450AFNG motor drivers    |
                              | BNO055 IMU                    |
                              | INA260 power monitor          |
                              | MAXSONAR obstacle sensor      |
                              | Speaker / AUX audio output    |
                              | Wireless charging receiver    |
                              +------------------------------+
```

## Hardware/Software Partitioning

A major design goal was to decide which parts of the system should be hardware and which parts should be software.

| Subsystem | Implemented In | Reason |
|---|---|---|
| Video data movement | FPGA fabric / AXI VDMA | High-throughput frame movement is better handled by dedicated hardware |
| HDMI output | FPGA video pipeline | Provides live visual debugging without CPU rendering |
| Motor PWM | Custom FPGA IP | PWM timing should be deterministic and independent of software jitter |
| Motor commands | ARM software | High-level decisions are easier to modify in C/C++ |
| Lane control | ARM software using camera frame data | Current implementation is easier to debug and tune in software |
| Future lane preprocessing | FPGA fabric | Thresholding, filtering, and lane feature extraction can be accelerated |
| IMU telemetry | ARM software over I2C | Sensor setup and polling are manageable in software |
| Power telemetry | ARM software over I2C | Voltage/current reporting is periodic, not cycle-critical |
| Sonar parsing | ARM software over UARTLite | ASCII range frames are simple to parse in software |
| Audio output | FPGA audio IP + ARM control | Audio streaming uses hardware IP while software triggers feedback |
| Robot state machine | ARM software | Behavior logic changes frequently during testing |
| Wireless charging demonstration | Power hardware + ARM telemetry | Charging behavior is verified through monitored voltage/current |

## Hardware Platform

### Main Compute Platform

- **Board:** Digilent Zybo Z7-20
- **SoC:** Xilinx Zynq-7000
- **Processor:** Dual-core ARM Cortex-A9
- **Programmable Logic:** Xilinx 7-series FPGA fabric
- **Development tools:** Vivado 2024.2 and Vitis 2024.2
- **Programming/debug interface:** JTAG and UART through the Zybo PROG UART micro-USB port

### Robot Chassis and Drive System

- **Chassis:** Four-wheel rover chassis based on the OSOYOO FlexiRover-style platform
- **Motors:** Four 6 V brushed DC TT motors
- **Drive style:** Differential / tank drive
- **Motor drivers:** TB67H450AFNG motor-driver modules
- **Control signal style:** PWM plus direction/brake control

Motor channel mapping:

| Motor Channel | Physical Motor |
|---|---|
| Motor 0 | Front left |
| Motor 1 | Back left |
| Motor 2 | Front right |
| Motor 3 | Back right |

### Sensors and Peripherals

| Device | Interface | Purpose |
|---|---|---|
| PCam 5C | MIPI CSI-2 | Camera input for lane following and dock detection |
| BNO055 | PS I2C1 | IMU telemetry and heading/motion experiments |
| INA260 | PS I2C1 | Battery voltage, current, and power telemetry |
| MAXSONAR | AXI UARTLite | Obstacle detection |
| SSM2603 codec | AXI IIC / I2S / Audio Formatter | Audio feedback |
| TB67H450AFNG drivers | FPGA PWM/GPIO-style outputs | Motor actuation |
| Wireless charging receiver | Power board | Charging demonstration |

## Vivado FPGA Design

The Vivado block design combines Digilent's camera/HDMI infrastructure, Xilinx AXI peripherals, and custom motor-control logic.

Major blocks include:

- Zynq7 Processing System
- MIPI CSI-2 receiver
- MIPI D-PHY receiver
- AXI VDMA
- Video timing controller
- Dynamic video clocking
- HDMI output path
- Custom `motor_ctrl_4ch` AXI4-Lite peripheral
- AXI UARTLite for sonar
- AXI Audio Formatter
- I2S Transmitter
- AXI IIC for codec configuration
- AXI GPIO for codec mute control
- AXI interconnects
- Processor system reset blocks
- PS I2C0 for camera configuration
- PS I2C1 for sensor telemetry

The Vivado design went through several incremental versions. The project was built up in phases because adding every subsystem at once makes debugging extremely difficult. The typical progression was:

1. Camera and HDMI passthrough only
2. Camera plus custom motor IP
3. Camera, motor IP, and I2C sensor bus
4. Camera, motor IP, sensors, and sonar UART
5. Audio subsystem added through Audio Formatter and I2S
6. FreeRTOS integration with concurrent tasks
7. Line-following control loop using the video frame buffer

## Embedded Software Architecture

The embedded software runs on the ARM Cortex-A9 processing system. The project uses standalone Vitis applications and FreeRTOS-based applications rather than a full Linux/PetaLinux system for the current integration work.

The software is responsible for:

- Initializing the platform
- Configuring the video pipeline
- Initializing motor-control registers
- Initializing I2C sensors
- Initializing UARTLite sonar
- Initializing the audio codec and audio IP
- Running the FreeRTOS scheduler
- Processing camera frame data
- Computing lane error
- Applying motor commands
- Handling obstacle stop behavior
- Printing telemetry over UART
- Managing robot state

### FreeRTOS Task Model

The integrated robot firmware is organized into multiple FreeRTOS tasks.

| Task | Purpose |
|---|---|
| Logger task | Centralizes UART output so multiple tasks do not directly fight over `xil_printf` |
| Sensor task | Initializes and reads BNO055 and power telemetry |
| Sonar task | Reads and parses MAXSONAR range frames |
| Vision task | Processes camera frame data and updates lane state |
| Motor task | Converts lane/obstacle state into motor-control outputs |
| Telemetry task | Periodically prints runtime state |
| Audio task | Initializes codec/audio path and plays feedback sounds when triggered |

This structure keeps subsystems isolated. If a sensor is unavailable, the rest of the robot can continue running instead of blocking the entire system.

## Camera and Video Pipeline

The video pipeline began from the Digilent Zybo Z7 PCam 5C HDMI passthrough design and was adapted into the autonomous rover system.

The camera pipeline performs the following:

1. Configure the OV5640 camera sensor over PS I2C.
2. Receive image data through MIPI CSI-2 and MIPI D-PHY.
3. Move frames into DDR memory using AXI VDMA.
4. Use multiple frame buffers for stable video movement.
5. Read frames back for HDMI output.
6. Allow software to inspect the frame buffer for line-following logic.
7. Draw debug overlays into the frame buffer so the HDMI output shows internal vision state.

Current frame assumptions:

```text
Resolution:       1920 x 1080
Pixel format:     RGB24
Bytes per pixel:  3
Frame buffers:    3
Base address:     DDR base + 0x0A000000
```

Typical runtime output:

```text
VDMA Frame 0 Addr: 0x0A000000
VDMA Frame 1 Addr: 0x0A5EEC00
VDMA Frame 2 Addr: 0x0ABDD800
```

The HDMI output is important because it provides a live view of what the robot is seeing. It also makes line-following debug much easier because the firmware can draw reference lines directly into the displayed frame.

## Line-Following Algorithm

The current line-following implementation reads from the live video frame buffer and searches for a tape-based lane in the lower part of the image.

The algorithm intentionally stays simple and explainable:

1. Select a region of interest near the bottom of the frame.
2. Sample pixels at a fixed x/y stride.
3. Compute a simple luma/brightness value.
4. Classify pixels as line pixels based on a threshold.
5. Accumulate the x-position of all detected line pixels.
6. Compute the average x-position of the detected line.
7. Compare the detected lane center to the center of the image.
8. Convert the pixel error into a steering correction.
9. Apply different duty cycles to the left and right motors.
10. Draw a visual overlay onto the HDMI output.

Conceptual control logic:

```text
lane_error_px = detected_lane_center_x - image_center_x

steering_delta = lane_error_px * Kp

left_duty  = base_duty + steering_delta
right_duty = base_duty - steering_delta
```

If the detected line is to the right of the image center, the left motors are driven faster than the right motors so the robot steers right. If the line is to the left, the right motors are driven faster than the left motors.

If no valid line is detected, the robot enters a search/stop behavior rather than blindly continuing forward.

### HDMI Debug Overlay

The firmware draws vertical reference lines onto the video stream:

- One line marks the image center.
- One line marks the detected lane center.

This makes it possible to visually confirm whether the robot is correctly detecting the lane and whether the control loop is trying to center the tape in the frame.

## Custom Motor-Control IP

The project includes a custom AXI4-Lite motor controller named `motor_ctrl_4ch`.

This IP exists because motor PWM should not depend on software timing. The ARM processor writes configuration and duty-cycle values over AXI, and the FPGA fabric generates the actual PWM outputs.

### Register Map

```text
Offset  Register      Description
0x00    REG_CTRL      Enable, direction, and brake bits
0x04    REG_PERIOD    PWM period
0x08    REG_DUTY0     Motor 0 duty cycle
0x0C    REG_DUTY1     Motor 1 duty cycle
0x10    REG_DUTY2     Motor 2 duty cycle
0x14    REG_DUTY3     Motor 3 duty cycle
```

### Control Bit Layout

```text
Enable bits:     [3:0]
Direction bits:  [7:4]
Brake bits:      [11:8]
```

Each motor channel supports:

- Enable
- Direction
- Brake
- Independent duty cycle

This allows the rover to control all four motors independently while keeping the PWM timing deterministic.

### Differential Drive Usage

The robot is controlled as a differential-drive vehicle:

- Motors 0 and 1 are the left side.
- Motors 2 and 3 are the right side.
- Steering is performed by changing the relative duty cycle between the two sides.

Example behavior:

```text
Drive straight: left duty ~= right duty
Turn right:     left duty > right duty
Turn left:      right duty > left duty
Stop:           all duties = 0
Brake:          brake bits asserted
```

## Sensor Integration

### BNO055 IMU

The BNO055 is connected to PS I2C1. It is used for motion telemetry and navigation experiments.

The firmware initializes the BNO055 by:

1. Waiting for the sensor to boot.
2. Reading the chip ID.
3. Switching to configuration mode.
4. Setting power mode.
5. Selecting units.
6. Switching into the desired operating mode.
7. Periodically reading acceleration and gyroscope data.

The IMU is useful for:

- Heading estimation
- Straight-line correction experiments
- Motion telemetry
- Square-path navigation tests
- Detecting drift or unexpected motion

### INA260 Power Monitor

The INA260 shares PS I2C1 with the BNO055 and is used to monitor the robot's electrical state.

It provides:

- Bus voltage
- Current
- Power

This is especially important for the wireless charging demonstration. When the robot reaches the charging dock, the system can observe battery voltage/current behavior to verify that charging is occurring.

### MAXSONAR Obstacle Sensor

The sonar sensor is connected through AXI UARTLite. It sends ASCII range frames, which are parsed by the sonar task.

The firmware reads frames in the format:

```text
R###\r
```

The parsed range is converted into inches and compared against a stop threshold.

Typical stop threshold:

```text
SONAR_STOP_INCHES = 18
```

## Obstacle Detection and Safe Stop

Obstacle detection is safety-critical because the robot should not continue driving if something is directly in its path.

The behavior priority is:

1. **Obstacle stop**
2. **Lane following**
3. **Search/hold if no lane is found**

The motor task gives obstacle detection the highest priority. If sonar reports an obstacle inside the stop threshold, the robot brakes regardless of the lane-following result.

Conceptual logic:

```text
if obstacle_detected:
    brake_all_motors()
    mode = OBSTACLE_STOP
else if lane_valid:
    follow_lane()
    mode = FOLLOWING
else:
    brake_all_motors()
    mode = SEARCHING
```

This is an important design point because the robot should fail safely. Losing the line or detecting an obstacle should not cause uncontrolled motion.

## Audio Feedback System

The rover includes an audio feedback system using the Zybo Z7 onboard audio path.

The final audio direction uses:

- SSM2603 codec
- AXI IIC for codec configuration
- AXI GPIO for mute control
- AXI Audio Formatter
- I2S Transmitter
- 12.288 MHz audio master clock
- 48 kHz sample rate

The purpose of audio is to provide audible feedback for robot states such as:

- Startup complete
- Obstacle detected
- System alert
- Charging event
- Test mode selected
- Debug tone or melody playback

The audio subsystem went through several iterations. Early testing explored direct I2S and DMA-style playback. The more stable design uses the Xilinx Audio Formatter with I2S TX. The firmware also uses bounded-timeout codec writes so the audio task cannot hang forever if codec I2C communication fails.

## Power Monitoring and Wireless Charging

The robot includes a wireless charging demonstration subsystem. The goal is to show that the robot can stop at a designated charging location and observe charging behavior through measured voltage/current telemetry.

The charging system includes:

- A charging station with a transmit-side coil and resonator
- A receive-side coil mounted on the robot
- Battery charging and management circuitry
- Power conversion circuitry
- INA260 telemetry for voltage/current/power observation
- Dock firmware and PCB files

The intended full behavior is:

1. Robot follows its lane or route.
2. Robot monitors battery state.
3. Low battery triggers a dock-seeking behavior.
4. Robot identifies the charging station using markers or a known route.
5. Robot aligns with the charging location.
6. Robot stops at the dock.
7. Wireless charging begins.
8. Voltage/current telemetry confirms the charging state.

## Repository Structure

```text
FPGA-Accelerated-Autonomous-Rover/
├── Charging_Dock_Firmware/
│   └── Firmware for the wireless charging dock controller
│
├── Data_Sheets/
│   └── Datasheets for major ICs, modules, boards, sensors, and power components
│
├── HDMI_Camera_Passthrough/
│   └── Camera and HDMI passthrough project files
│
├── PCB_Files/
│   └── PCB design files for power, charging, and peripheral boards
│
├── Source_Files_for_Vitis/
│   └── Embedded C/C++ firmware, subsystem tests, and FreeRTOS integration code
│
├── Useful_bitstreams/
│   └── Known-good FPGA bitstreams for different hardware configurations
│
├── vivado-library-master/
│   └── Digilent/Vivado library assets used by the video/camera design
│
├── How to Fix PCAM vitis code.md
│   └── Notes for adapting and fixing PCam-related Vitis code
│
└── README.md
```

## Important Source Files

Representative firmware and support files include:

| File | Purpose |
|---|---|
| `rtos_line_follow_debug.cc` | FreeRTOS line-following application with video, sonar, BNO055 telemetry, motor control, and HDMI overlay |
| `RTOS_test_no_audio.cc` | FreeRTOS integration test without audio |
| `audio_rtos_tests.cc` | FreeRTOS integration test with audio initialization and beep playback |
| `rtos_square_path.cc` | Square-path navigation experiment using IMU, sonar, and motor control |
| `main.cc` | Earlier standalone camera/video passthrough or integration source |
| `main.c` | Selectable subsystem test firmware |
| `motor_ctrl_4ch.h` | Software driver header for the custom motor-control IP |
| `motor_ctrl_4ch.c` | Motor-control IP driver source |
| `motor_ctrl_4ch_selftest.c` | Register read/write self-test for the motor-control IP |
| `xparams_compat.h` | Compatibility header for SDT-style `xparameters.h` differences |
| `assert_filename_fix.h` | Build compatibility helper |
| `OV5640.h/.cpp` | OV5640 camera configuration support |
| `AXI_VDMA.h` | C++ wrapper for the Xilinx AXI VDMA driver |
| `PS_IIC.h` | PS I2C driver wrapper |
| `PS_GPIO.h` | PS GPIO driver wrapper |
| `ScuGicInterruptController.h` | GIC interrupt-controller wrapper |

## Build Environment

This project was developed using:

- **Vivado:** 2024.2
- **Vitis:** 2024.2
- **Target board:** Digilent Zybo Z7-20
- **Target SoC:** Xilinx Zynq-7000
- **Firmware style:** Standalone and FreeRTOS Vitis applications
- **Programming method:** JTAG through the Zybo PROG UART micro-USB port
- **Debug terminal:** UART console through PuTTY or equivalent serial terminal

## Bring-Up and Test Strategy

The project was brought up incrementally because the complete system contains many interacting peripherals. Testing each block independently made it easier to isolate hardware, Vivado, Vitis, and runtime problems.

Recommended bring-up order:

### 1. Video Only

- Confirm PCam 5C is detected.
- Confirm OV5640 camera configuration over I2C.
- Confirm HDMI output.
- Confirm AXI VDMA frame addresses.
- Confirm stable live video.

### 2. Motor IP Only

- Confirm the custom AXI motor IP appears in `xparameters.h`.
- Confirm the base address matches Vivado Address Editor.
- Write the PWM period register.
- Test one motor channel at a time.
- Verify forward, reverse, stop, and brake behavior.
- Confirm TB67H450AFNG wiring and motor polarity.

### 3. BNO055 Only

- Confirm PS I2C1 wiring.
- Confirm the BNO055 chip ID.
- Confirm acceleration and gyroscope reads.
- Verify that the sensor does not block the scheduler if unavailable.

### 4. INA260 Only

- Confirm shared I2C1 bus behavior.
- Confirm manufacturer and die ID reads.
- Confirm bus voltage, current, and power readings.
- Verify telemetry during charging tests.

### 5. Sonar Only

- Confirm AXI UARTLite base address.
- Confirm UARTLite self-test.
- Confirm ASCII range frame parsing.
- Confirm obstacle threshold behavior.

### 6. Audio Only

- Confirm audio master clock.
- Confirm mute GPIO.
- Confirm codec I2C writes.
- Confirm I2S TX setup.
- Confirm Audio Formatter setup.
- Confirm tone or jingle playback.
- Confirm bounded-timeout behavior prevents lockups.

### 7. FreeRTOS Integration

- Start with logger and one subsystem.
- Add sensor task.
- Add sonar task.
- Add motor task.
- Add telemetry task.
- Add vision task.
- Add audio task only after the core runtime is stable.
- Monitor stack high-water marks and heap usage.

### 8. Full Robot Test

- Confirm video passthrough.
- Confirm HDMI overlay.
- Confirm lane center detection.
- Confirm motor response.
- Confirm obstacle stop.
- Confirm telemetry.
- Confirm charging/power telemetry.
- Confirm audio feedback.

## Typical UART Output

Example output from the integrated FreeRTOS line-following application:

```text
========================================
FreeRTOS line-following robot
Video base  = 0x0A000000 (3 frames, RGB24 1920x1080)
Motor base  = 0x43C50000
UARTLite    = 0x42C00000
[video] passthrough init ok
[rtos] starting scheduler
[sensor] init start
[sonar] init start
[motor] init start
[vision] detection active, overlay visible on HDMI
[motor] line-follow control active
```

Example telemetry output:

```text
[telemetry] video=1 bno=1 sonar=1 lane=1 center=944 err=-16 hits=120 mode=FOLLOW left=692 right=708 range=31 in
```

The runtime telemetry is useful because it shows whether each major subsystem is alive:

```text
video = video passthrough initialized
bno   = BNO055 telemetry available
sonar = sonar range frames being received
lane  = valid lane currently detected
err   = lane offset from image center
mode  = robot drive state
left  = left-side motor duty
right = right-side motor duty
range = sonar distance
```

## Engineering Challenges and Fixes

### Camera Pipeline Bring-Up

The camera pipeline required combining the PCam 5C, MIPI CSI-2, D-PHY, VDMA, dynamic video clocking, and HDMI output path. The initial goal was to reproduce the Digilent passthrough behavior, then integrate that pipeline into the larger robot system.

Important lessons:

- The video pipeline must be stable before adding robot-control logic.
- Frame buffer addresses are one of the first things to verify.
- HDMI output is extremely useful for debugging perception.
- The software must respect the actual frame format and stride.
- Cache coherency matters when software modifies frame buffers that VDMA reads.

### Vitis and `xparameters.h` Compatibility

Some Digilent example code expected classic `xparameters.h` macro names, while newer Vitis/SDT-style platforms may generate different names. The `xparams_compat.h` helper was added to map missing or renamed macros so older-style code could build against the generated platform.

This reduced friction when integrating Digilent camera code with newer Vitis-generated BSP/platform files.

### FreeRTOS and Interrupt Conflicts

One of the major stability issues was getting the video stack and FreeRTOS runtime to coexist. The video demo code and the FreeRTOS Zynq port both interact with interrupt-controller setup. If the video pipeline leaves certain interrupts active while FreeRTOS is taking over scheduling, the runtime can become unstable.

The practical fix was to keep the video passthrough running in hardware while masking or disabling live PL-to-PS video interrupt sources before handing control to FreeRTOS. This allowed the camera/HDMI path to continue while the robot tasks ran under the scheduler.

### Sensor Initialization Blocking

Early versions of the firmware could block forever if a sensor did not ACK on I2C. This is dangerous in a robot runtime because one missing or disconnected device should not prevent the entire system from booting.

The better pattern is:

- Try initialization a bounded number of times.
- Mark the sensor unavailable if initialization fails.
- Continue running the rest of the system.
- Print telemetry showing which subsystem is unavailable.

This approach makes the robot more fault-tolerant during hardware bring-up.

### UART Logging From Multiple Tasks

Direct `xil_printf` calls from multiple FreeRTOS tasks can make logs unreadable and can create debugging confusion. The integrated firmware uses a logger task and log queue so most runtime messages pass through a centralized print path.

This improves:

- UART readability
- Task isolation
- Debug consistency
- Runtime stability

### Audio Integration

Audio integration was challenging because it involved the SSM2603 codec, I2C configuration, mute control, clocking, I2S TX, and Audio Formatter behavior.

Important fixes included:

- Using AXI Audio Formatter with I2S TX
- Providing a 12.288 MHz audio master clock for 48 kHz audio
- Carefully controlling codec mute
- Avoiding unbounded I2C waits
- Using bounded-timeout codec register writes
- Avoiding direct UART prints from the audio task

### Motor Control and Duty Tuning

Motor behavior depends heavily on the physical chassis, motor friction, battery voltage, floor surface, and wheel slip. The motor controller supports independent channel duty values, but the actual line-following behavior requires tuning:

- Base duty
- Minimum duty
- Maximum duty
- Steering proportional gain
- Direction polarity
- Brake behavior
- Left/right motor balance

The project uses configurable constants for these values so the rover can be tuned quickly during testing.

## Current Status

The current project status includes:

- Camera passthrough working through PCam 5C and HDMI
- VDMA frame buffering working with 1920x1080 RGB frames
- Custom motor-control IP integrated and tested
- FreeRTOS runtime integrated with multiple tasks
- Line-following firmware implemented using frame-buffer analysis
- HDMI debug overlay implemented
- BNO055 telemetry integrated
- INA260 power monitoring integrated in test firmware
- Sonar-based obstacle detection implemented
- Audio subsystem tested through Audio Formatter and I2S
- Wireless charging dock firmware and PCB files included
- Multiple subsystem test files available for bring-up and debugging

## Known Limitations

This is an active senior design project, so some parts are still experimental or hardware-dependent.

Known limitations:

- The current lane-detection algorithm depends on lighting, floor color, tape contrast, and camera angle.
- The line-following algorithm is currently simple threshold-based detection.
- More image preprocessing could be moved into FPGA fabric in future revisions.
- Dock detection may require additional AprilTag or visual-marker logic.
- Wireless charging depends heavily on coil alignment and power-board behavior.
- Vitis projects may require local path or platform adjustments after cloning.
- Hardware base addresses can change if the Vivado block design changes.
- Some source files are subsystem tests rather than one final application.
- Full autonomous low-battery docking behavior is still a future integration goal.

## Future Work

Planned improvements include:

- Move more image preprocessing into FPGA fabric.
- Add a cleaner lane-feature extraction hardware block.
- Improve lane detection under different lighting conditions.
- Add adaptive thresholding for tape detection.
- Add AprilTag or visual-marker dock identification.
- Implement a complete dock approach and alignment state machine.
- Use INA260 telemetry for battery-state estimation.
- Add low-battery behavior that triggers autonomous docking.
- Improve closed-loop heading correction using the BNO055.
- Add QSPI boot support for power-on autonomous startup.
- Clean up source organization into reusable drivers and application layers.
- Add more complete Vivado reconstruction documentation.
- Add final demo videos and system diagrams.
- Add measured timing, latency, and power results.

## Skills Demonstrated

This project demonstrates:

- FPGA/SoC hardware/software co-design
- Zynq-7000 Processing System and Programmable Logic integration
- Vivado block design development
- Vitis embedded software development
- AXI4-Lite custom IP development
- Memory-mapped peripheral control
- Deterministic PWM generation in FPGA fabric
- MIPI camera integration
- HDMI video output
- AXI VDMA frame buffering
- FreeRTOS task design
- Embedded C/C++ firmware development
- I2C, UART, GPIO, I2S, and DMA-style peripheral integration
- Sensor bring-up and debugging
- Audio codec configuration
- Real-time telemetry logging
- Robotics control logic
- Obstacle detection and safe-stop behavior
- PCB-level system integration
- Wireless charging demonstration
- Incremental hardware/software validation
- Debugging with UART traces and hardware isolation tests

## Team

**Team Name:** Justice League

- **Sanat Konda**
- **Ryan Robinson**

University of Florida  
Electrical and Computer Engineering  
EEL 4924 Senior Design

## Project Summary

The FPGA-Accelerated Autonomous Rover is a complete embedded robotics system built around a Zynq FPGA SoC. It combines live video, custom motor-control hardware, FreeRTOS-based software, sensor telemetry, obstacle detection, audio feedback, and wireless charging into one autonomous mobile robot platform.

The project demonstrates the core idea of hardware/software co-design: not every task should be forced through software. Timing-sensitive and deterministic functions are better placed in FPGA fabric, while high-level robot behavior is better handled by software running on the ARM processor.

This makes the rover a practical demonstration of SoC architecture, embedded robotics, custom FPGA peripherals, real-time software, and full-system integration.
