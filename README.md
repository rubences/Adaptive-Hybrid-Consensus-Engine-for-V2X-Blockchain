# Adaptive Hybrid Consensus Engine for V2X Blockchain

Implementation of "Adaptive Hybrid Consensus Engine for V2X Blockchain: Real-Time Entropy-Driven Control for High Energy Efficiency and Sub-100 ms Latency"

## Overview

This project implements a governance layer for VANETs (Vehicular Ad-hoc Networks) that dynamically switches between consensus mechanisms based on network disorder (entropy). The system runs as an application on top of IEEE 802.11p (WAVE) in NS-3 (Network Simulator 3).

### Key Features

- **Entropy-Driven Consensus Switching**: Automatically switches between Low-Cost (PoW) and High-Rigor (PoS/FBA) modes based on:
  - **Informational Entropy (S)**: Measures transaction distribution across nodes
  - **Spatial Entropy (H_spatial)**: Measures vehicle dispersion in the network
  
- **Multiple Consensus Mechanisms**:
  - **PoW (Proof-of-Work)**: Low-cost mode with hash-based mining
  - **PoS (Proof-of-Stake)**: High-rigor mode with validator selection
  - **FBA (Federated Byzantine Agreement)**: Quorum-based consensus
  
- **Quality of Information (QoI) Filtering**: Filters transactions based on delay thresholds
  
- **Energy Modeling**: Hybrid energy model combining:
  - Radio energy (NS-3 WifiRadioEnergyModel)
  - Cryptographic energy (analytical: E_crypto = e_h × n_hash or e_sig × n_sig)

## Architecture

### Core Components

```
src/v2x-engine/
├── model/
│   ├── consensus-module.{h,cc}      # Abstract base class for consensus
│   ├── pow-module.{h,cc}            # Proof-of-Work implementation
│   ├── pos-module.{h,cc}            # Proof-of-Stake implementation
│   ├── fba-module.{h,cc}            # Federated Byzantine Agreement
│   └── node-app.{h,cc}              # NS-3 Application for vehicle nodes
└── helper/
    └── vanet-engine-helper.{h,cc}   # Main control loop with entropy calculation
```

### Control Logic

The system implements the following decision logic:

```
If (S > S_th) OR (H_spatial > H_th):
    Switch to HIGH_RIGOR mode (PoS/FBA)
Else:
    Switch to LOW_COST mode (PoW)
```

Where:
- S_th = 0.5 (Informational entropy threshold)
- H_th = 0.6 (Spatial entropy threshold)

### Information Cycle

The system follows an "Ideal Information Cycle":
1. **Injection**: Collect pending transactions from all nodes
2. **Validation**: Apply QoI filtering and consensus validation
3. **Commit**: Add validated blocks to the blockchain ledger

## Prerequisites

- **NS-3** version 3.35 or later
- **C++17** compatible compiler (g++ 7.0+ or clang++ 5.0+)
- **Python 3.6+** for analysis scripts
- **Python packages**: pandas, matplotlib, numpy

## Installation

### 1. Install NS-3

```bash
# Download NS-3 (if not already installed)
wget https://www.nsnam.org/releases/ns-allinone-3.35.tar.bz2
tar xjf ns-allinone-3.35.tar.bz2
cd ns-allinone-3.35/ns-3.35/
```

### 2. Clone this repository into NS-3 source

```bash
# Clone into NS-3 src directory
cd src/
git clone https://github.com/rubences/Adaptive-Hybrid-Consensus-Engine-for-V2X-Blockchain.git v2x-engine
cd ..

# Or copy the v2x-engine module
cp -r /path/to/this/repo/src/v2x-engine ./src/
```

### 3. Copy simulation scripts

```bash
# Copy simulation scenarios to scratch directory
cp src/v2x-engine/scratch/*.cc scratch/
```

### 4. Configure and build NS-3

```bash
# Configure NS-3 with examples enabled
./ns3 configure --enable-examples --enable-tests

# Build the project
./ns3 build
```

## Usage

### Running Simulations

#### Urban Grid Scenario

Simulates vehicles in an urban environment with random waypoint mobility:

```bash
./ns3 run "v2x-urban-sim --nNodes=50 --simTime=100 --gridSize=1000 --nodeSpeed=15"
```

**Parameters**:
- `--nNodes`: Number of vehicle nodes (default: 50)
- `--simTime`: Simulation duration in seconds (default: 100)
- `--gridSize`: Size of urban grid in meters (default: 1000)
- `--nodeSpeed`: Average vehicle speed in m/s (default: 15)
- `--beaconInterval`: CAM beacon interval in seconds (default: 0.1)
- `--logFile`: Output CSV file (default: v2x-urban-metrics.csv)

#### Highway Scenario

Simulates vehicles on a multi-lane highway with constant velocity:

```bash
./ns3 run "v2x-highway-sim --nNodes=80 --simTime=100 --highwayLength=5000 --nLanes=3"
```

**Parameters**:
- `--nNodes`: Number of vehicle nodes (default: 80)
- `--simTime`: Simulation duration in seconds (default: 100)
- `--highwayLength`: Highway length in meters (default: 5000)
- `--nLanes`: Number of highway lanes (default: 3)
- `--nodeSpeed`: Average vehicle speed in m/s (default: 30)
- `--beaconInterval`: CAM beacon interval in seconds (default: 0.1)
- `--logFile`: Output CSV file (default: v2x-highway-metrics.csv)

### Analyzing Results

#### Install Python dependencies

```bash
pip install pandas matplotlib numpy
```

#### Run analysis script

```bash
cd analysis/
python analyze_results.py ../v2x-urban-metrics.csv ./urban_results
python analyze_results.py ../v2x-highway-metrics.csv ./highway_results
```

The analysis script generates:
- **latency_vs_entropy.png**: Latency correlation with informational and spatial entropy
- **energy_efficiency.png**: Energy consumption breakdown and efficiency metrics
- **entropy_timeline.png**: Evolution of entropy values over time
- **ledger_divergence.png**: Fork events and consensus mode distribution

## Output Metrics

The simulation generates a CSV file with the following columns:

| Column | Description |
|--------|-------------|
| Time | Simulation time (seconds) |
| NodeID | Node identifier |
| InformationalEntropy | S(t) - Transaction distribution entropy |
| SpatialEntropy | H_spatial(t) - Vehicle dispersion entropy |
| Mode | Current consensus mode (LOW_COST or HIGH_RIGOR) |
| RadioEnergy | Energy consumed by radio (Joules) |
| CryptoEnergy | Energy consumed by cryptographic operations (Joules) |
| Latency | Consensus latency (seconds) |
| ForkEvent | Whether a fork occurred (0 or 1) |

## Configuration

### Entropy Thresholds

Modify thresholds in simulation scripts or at runtime:

```cpp
vanetEngine->SetInformationalEntropyThreshold(0.5);  // Default: 0.5
vanetEngine->SetSpatialEntropyThreshold(0.6);        // Default: 0.6
```

### Block Parameters

```cpp
vanetEngine->SetBlockSize(10);              // Transactions per block
vanetEngine->SetBeaconInterval(0.1);        // 100ms beacon interval
```

### Energy Parameters

```cpp
// PoW energy per hash (Joules)
powModule->SetEnergyPerHash(5e-6);          // Default: 5 μJ

// PoS/FBA energy per signature (Joules)
posModule->SetEnergyPerSignature(1e-6);     // Default: 1 μJ
fbaModule->SetEnergyPerSignature(1e-6);
```

## Design Principles

### Entropy Calculation

**Informational Entropy (S)**:
```
S = -Σ p_i × log₂(p_i)
```
where p_i is the fraction of pending transactions at node i.

**Spatial Entropy (H_spatial)**:
```
H_spatial = -Σ (n_i/N) × log₂(n_i/N)
```
where n_i is the number of vehicles in grid cell i, N is total vehicles.

### Energy Model

**PoW Energy**:
```
E_crypto = e_h × n_hash
```
where e_h ≈ 5 μJ/hash, n_hash follows exponential distribution based on difficulty.

**PoS/FBA Energy**:
```
E_crypto = e_sig × n_sig
```
where e_sig ≈ 1 μJ/signature, n_sig is number of signatures required.

## Performance Targets

Based on the reference paper:
- **Latency**: Sub-100 ms for block generation and validation
- **Energy Efficiency**: Adaptive switching reduces energy consumption by 30-40%
- **Fork Rate**: Minimized through high-rigor mode during high entropy periods

## Testing

```bash
# Run basic functionality tests
./ns3 run v2x-urban-sim --duration=10 --nNodes=10

# Verify output file generation
ls -l v2x-urban-metrics.csv

# Quick analysis
python analysis/analyze_results.py v2x-urban-metrics.csv
```

## Troubleshooting

### Build Issues

If you encounter build errors:

```bash
# Clean and rebuild
./ns3 clean
./ns3 configure --enable-examples
./ns3 build
```

### Missing Dependencies

For WAVE/802.11p support:
```bash
# Ensure WAVE module is enabled in NS-3
./ns3 configure --enable-examples --enable-modules=wave
```

### Python Analysis Errors

```bash
# Install all required packages
pip install -r analysis/requirements.txt
```

## Project Structure

```
.
├── src/v2x-engine/           # Main module source code
│   ├── model/                # Core consensus and application models
│   ├── helper/               # VANET engine helper
│   └── wscript               # Build configuration
├── scratch/                  # Simulation scenarios
│   ├── v2x-urban-sim.cc      # Urban grid scenario
│   └── v2x-highway-sim.cc    # Highway scenario
├── analysis/                 # Python analysis scripts
│   └── analyze_results.py    # Result visualization
└── README.md                 # This file
```

## Reference

This implementation is based on the research paper:

**"Adaptive Hybrid Consensus Engine for V2X Blockchain: Real-Time Entropy-Driven Control for High Energy Efficiency and Sub-100 ms Latency"**

Key contributions:
- Entropy-driven consensus mode switching
- Hybrid energy modeling (radio + cryptographic)
- Sub-100 ms latency achievement
- Quality of Information filtering

## License

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 2 as published by the Free Software Foundation.

## Authors

V2X Blockchain Research Group

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Submit a pull request

## Citation

If you use this code in your research, please cite:

```bibtex
@article{v2x-adaptive-consensus,
  title={Adaptive Hybrid Consensus Engine for V2X Blockchain: Real-Time Entropy-Driven Control for High Energy Efficiency and Sub-100 ms Latency},
  author={V2X Research Group},
  year={2024}
}
```

## Contact

For questions or issues, please open an issue on GitHub or contact the maintainers.
