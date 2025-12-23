# Quick Start Guide

This guide will help you get started with the Adaptive Hybrid Consensus Engine for V2X Blockchain quickly.

## Prerequisites Check

Before starting, ensure you have:

- [ ] NS-3 3.35 or later installed
- [ ] C++17 compatible compiler (g++ 7.0+)
- [ ] Python 3.6+
- [ ] Git

## 5-Minute Setup

### Step 1: Install NS-3 (if not already installed)

```bash
# Download NS-3
wget https://www.nsnam.org/releases/ns-allinone-3.35.tar.bz2
tar xjf ns-allinone-3.35.tar.bz2
cd ns-allinone-3.35/ns-3.35/
```

### Step 2: Clone This Repository

Option A - As NS-3 module (recommended):
```bash
cd src/
git clone https://github.com/rubences/Adaptive-Hybrid-Consensus-Engine-for-V2X-Blockchain.git v2x-engine
cd ..
```

Option B - Standalone:
```bash
git clone https://github.com/rubences/Adaptive-Hybrid-Consensus-Engine-for-V2X-Blockchain.git
```

### Step 3: Copy Simulation Scripts

```bash
# From NS-3 root directory
cp src/v2x-engine/scratch/*.cc scratch/
```

### Step 4: Build

```bash
./ns3 configure --enable-examples
./ns3 build
```

Expected output:
```
Build finished successfully
```

### Step 5: Run Your First Simulation

```bash
./ns3 run v2x-urban-sim
```

You should see output like:
```
======================================
V2X Urban Grid Simulation
Number of nodes: 50
Simulation time: 100 s
Grid size: 1000 m
======================================
Starting simulation...
```

### Step 6: Analyze Results

```bash
cd analysis/
pip install -r requirements.txt
python analyze_results.py ../v2x-urban-metrics.csv
```

You should see:
- Summary statistics printed to console
- 4 PNG files generated in `analysis_results/`

## What's Next?

### Run Different Scenarios

**Urban with more vehicles:**
```bash
./ns3 run "v2x-urban-sim --nNodes=100 --simTime=200"
```

**Highway scenario:**
```bash
./ns3 run "v2x-highway-sim --nNodes=80 --nodeSpeed=30"
```

### Customize Parameters

Edit the simulation files in `scratch/` to adjust:
- Number of nodes
- Mobility patterns
- Energy parameters
- Entropy thresholds

Example:
```cpp
// In scratch/v2x-urban-sim.cc
vanetEngine->SetInformationalEntropyThreshold(0.6);  // Change from 0.5
vanetEngine->SetSpatialEntropyThreshold(0.7);        // Change from 0.6
```

### Explore the Code

Key files to understand:
1. `src/v2x-engine/helper/vanet-engine-helper.cc` - Main control logic
2. `src/v2x-engine/model/pow-module.cc` - Proof-of-Work implementation
3. `scratch/v2x-urban-sim.cc` - Urban simulation setup

## Common Commands

```bash
# Build only
./ns3 build

# Clean build
./ns3 clean
./ns3 build

# Run with logging
./ns3 run "v2x-urban-sim --verbose"

# Run with NS-3 logging
NS_LOG=VanetEngineHelper=level_info ./ns3 run v2x-urban-sim

# Analyze specific CSV file
python analysis/analyze_results.py my-custom-results.csv ./output
```

## Troubleshooting

### Build Errors

**Error: Cannot find v2x-engine module**
```bash
# Make sure the module is in NS-3 src directory
ls src/v2x-engine/
# Should show: model/ helper/ wscript
```

**Error: Missing WAVE module**
```bash
./ns3 configure --enable-examples --enable-modules=wave
```

### Runtime Errors

**Error: Cannot create socket**
- Check that you have proper network permissions
- Try running with sudo (not recommended) or fix network capabilities

**Segmentation fault**
- Check NS-3 version compatibility
- Ensure all dependencies are installed
- Try rebuilding: `./ns3 clean && ./ns3 build`

### Analysis Errors

**ModuleNotFoundError: No module named 'pandas'**
```bash
pip install pandas matplotlib numpy
```

**No output CSV file**
- Check that simulation completed successfully
- Look for error messages in simulation output
- Verify write permissions in current directory

## Getting Help

- Check the main [README.md](README.md) for detailed documentation
- Review [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines
- Open an issue on GitHub for bugs or questions
- Read the NS-3 documentation at https://www.nsnam.org/

## Next Steps

Once you're comfortable with the basics:

1. **Modify consensus parameters** to see effects on performance
2. **Create custom mobility scenarios** based on real traffic data
3. **Implement additional consensus mechanisms**
4. **Integrate with real V2X hardware** (if available)
5. **Contribute improvements** back to the project

## Resources

- [NS-3 Tutorial](https://www.nsnam.org/docs/tutorial/html/)
- [NS-3 Manual](https://www.nsnam.org/docs/manual/html/)
- [WAVE/802.11p Documentation](https://www.nsnam.org/docs/models/html/wave.html)
- [Project Wiki](https://github.com/rubences/Adaptive-Hybrid-Consensus-Engine-for-V2X-Blockchain/wiki)

Happy simulating! 🚗⛓️
