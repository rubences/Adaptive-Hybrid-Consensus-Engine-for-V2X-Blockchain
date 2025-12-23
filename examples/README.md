# V2X Blockchain Examples

This directory contains example configurations and use cases for the Adaptive Hybrid Consensus Engine.

## Available Examples

### 1. Basic Urban Simulation

**Description**: Simple urban grid with 50 vehicles

**Command**:
```bash
./ns3 run v2x-urban-sim
```

**Expected Output**: `v2x-urban-metrics.csv` with ~10,000 records

**Use Case**: Testing basic functionality and understanding system behavior

---

### 2. High-Density Urban

**Description**: Dense urban area with 100 vehicles, longer simulation time

**Command**:
```bash
./ns3 run "v2x-urban-sim --nNodes=100 --simTime=200 --gridSize=2000 --logFile=high-density-urban.csv"
```

**Expected Behavior**:
- Higher informational entropy (more transactions)
- Frequent mode switching to HIGH_RIGOR
- Sub-100ms latency in LOW_COST mode

**Use Case**: Evaluating scalability and mode switching behavior

---

### 3. Highway Traffic

**Description**: Highway scenario with 80 vehicles at high speeds

**Command**:
```bash
./ns3 run v2x-highway-sim
```

**Expected Output**: `v2x-highway-metrics.csv`

**Expected Behavior**:
- Lower spatial entropy (linear arrangement)
- More consistent vehicle positions
- Primarily LOW_COST mode operation

**Use Case**: Testing linear mobility patterns

---

### 4. Custom Threshold Testing

**Description**: Test different entropy thresholds

**Modification**: Edit `scratch/v2x-urban-sim.cc`:

```cpp
// Line ~120 - Modify thresholds
vanetEngine->SetInformationalEntropyThreshold(0.3);  // Lower threshold
vanetEngine->SetSpatialEntropyThreshold(0.4);        // Lower threshold
```

**Command**:
```bash
./ns3 build
./ns3 run "v2x-urban-sim --logFile=low-threshold.csv"
```

**Expected Behavior**:
- More frequent HIGH_RIGOR mode activation
- Lower latency overall
- Higher energy consumption

**Use Case**: Finding optimal thresholds for specific scenarios

---

### 5. Energy Efficiency Comparison

**Description**: Compare energy consumption across consensus modes

**Commands**:
```bash
# Baseline - normal thresholds
./ns3 run "v2x-urban-sim --logFile=normal-thresholds.csv"

# Always LOW_COST - set very high thresholds (modify code)
# SetInformationalEntropyThreshold(10.0)
./ns3 run "v2x-urban-sim --logFile=always-low-cost.csv"

# Always HIGH_RIGOR - set very low thresholds (modify code)
# SetInformationalEntropyThreshold(0.0)
./ns3 run "v2x-urban-sim --logFile=always-high-rigor.csv"
```

**Analysis**:
```bash
python analysis/analyze_results.py normal-thresholds.csv ./results_normal
python analysis/analyze_results.py always-low-cost.csv ./results_low
python analysis/analyze_results.py always-high-rigor.csv ./results_high
```

**Expected Findings**:
- Adaptive approach balances latency and energy
- Pure LOW_COST saves energy but increases latency during high entropy
- Pure HIGH_RIGOR wastes energy during low entropy periods

**Use Case**: Demonstrating adaptive algorithm benefits

---

### 6. Long-Running Stability Test

**Description**: Extended simulation to test stability

**Command**:
```bash
./ns3 run "v2x-urban-sim --nNodes=50 --simTime=1000 --logFile=long-run.csv"
```

**Expected Duration**: ~5-10 minutes

**Expected Output**: Large CSV file (~100K records)

**Use Case**: Testing for memory leaks, fork events, and long-term stability

---

## Parameter Reference

### Common Parameters

| Parameter | Default | Description | Valid Range |
|-----------|---------|-------------|-------------|
| `--nNodes` | 50 (urban) / 80 (highway) | Number of vehicles | 10-200 |
| `--simTime` | 100 | Simulation duration (s) | 10-3600 |
| `--gridSize` | 1000 | Urban grid size (m) | 500-5000 |
| `--nodeSpeed` | 15 (urban) / 30 (highway) | Avg speed (m/s) | 5-50 |
| `--beaconInterval` | 0.1 | CAM interval (s) | 0.01-1.0 |
| `--logFile` | v2x-urban-metrics.csv | Output file | any filename |

### Advanced Configuration

To modify advanced parameters, edit the source files:

**Entropy Thresholds** (`scratch/v2x-*-sim.cc`):
```cpp
vanetEngine->SetInformationalEntropyThreshold(0.5);
vanetEngine->SetSpatialEntropyThreshold(0.6);
```

**Block Parameters**:
```cpp
vanetEngine->SetBlockSize(10);           // Transactions per block
vanetEngine->SetBeaconInterval(0.1);     // 100ms
```

**Energy Costs** (`src/v2x-engine/model/pow-module.cc` etc.):
```cpp
m_energyPerHash = 5e-6;      // 5 μJ per hash (PoW)
m_energyPerSignature = 1e-6; // 1 μJ per signature (PoS/FBA)
```

## Creating Your Own Example

1. Copy an existing simulation:
```bash
cp scratch/v2x-urban-sim.cc scratch/my-custom-sim.cc
```

2. Modify parameters in `my-custom-sim.cc`

3. Build and run:
```bash
./ns3 build
./ns3 run my-custom-sim
```

4. Analyze results:
```bash
python analysis/analyze_results.py <output-file>.csv
```

## Tips for Experimentation

1. **Start small**: Use fewer nodes and shorter sim time for initial tests
2. **Change one parameter at a time**: Easier to understand effects
3. **Keep logs organized**: Use descriptive filenames
4. **Document changes**: Note what you modified in each run
5. **Compare results**: Use the analysis scripts to compare different configurations

## Validation Checklist

When running examples, verify:

- [ ] Simulation completes without errors
- [ ] CSV file is generated
- [ ] CSV contains expected columns
- [ ] Entropy values are in reasonable range (0-5)
- [ ] Latency is sub-100ms in most cases
- [ ] Both LOW_COST and HIGH_RIGOR modes are used (in adaptive scenarios)
- [ ] Analysis scripts run successfully
- [ ] Plots are generated correctly

## Common Issues

**Simulation crashes**: Reduce number of nodes or simulation time

**CSV file is empty**: Check for simulation errors in console output

**No mode switching**: Entropy may be consistently below/above thresholds

**All transactions filtered**: QoI delay threshold may be too strict

## Further Reading

- Main [README.md](../README.md) for architecture details
- [QUICKSTART.md](../QUICKSTART.md) for setup instructions
- NS-3 [documentation](https://www.nsnam.org/documentation/)
