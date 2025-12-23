# Analysis Scripts

This directory contains Python scripts for analyzing V2X blockchain simulation results.

## Requirements

Install required packages:

```bash
pip install -r requirements.txt
```

Or install individually:

```bash
pip install pandas matplotlib numpy
```

## Usage

### Basic Analysis

```bash
python analyze_results.py <csv_file> [output_directory]
```

Example:

```bash
python analyze_results.py ../v2x-urban-metrics.csv ./urban_results
```

### Output

The script generates the following visualizations:

1. **latency_vs_entropy.png**
   - Scatter plots showing correlation between latency and entropy
   - Separate plots for informational and spatial entropy
   - Threshold lines showing S_th and H_th

2. **energy_efficiency.png**
   - Energy consumption over time
   - Energy breakdown by consensus mode
   - Cumulative energy consumption
   - Energy efficiency per transaction

3. **entropy_timeline.png**
   - Time series of informational entropy
   - Time series of spatial entropy
   - Consensus mode switching visualization

4. **ledger_divergence.png**
   - Fork event timeline
   - Fork events by consensus mode

### Statistics

The script also prints comprehensive statistics including:
- Mean and median latency
- Energy consumption totals
- Entropy statistics
- Consensus mode distribution
- Fork event counts

## Examples

### Compare Urban vs Highway

```bash
# Analyze urban scenario
python analyze_results.py ../v2x-urban-metrics.csv ./urban_results

# Analyze highway scenario
python analyze_results.py ../v2x-highway-metrics.csv ./highway_results
```

### Custom Analysis

You can extend the script by adding your own analysis functions:

```python
def custom_analysis(df):
    # Your analysis code here
    pass

# Add to main()
custom_analysis(df)
```

## Data Format

Expected CSV format:

```
Time,NodeID,InformationalEntropy,SpatialEntropy,Mode,RadioEnergy,CryptoEnergy,Latency,ForkEvent
0.0,1,0.5,0.6,LOW_COST,0.001,0.0001,0.05,0
...
```

## Troubleshooting

### Import Errors

If you get import errors:

```bash
pip install --upgrade pandas matplotlib numpy
```

### Display Issues

For headless systems (no display):

```python
import matplotlib
matplotlib.use('Agg')  # Add at top of script
```

### Memory Issues

For large datasets:

```python
# Process in chunks
chunks = pd.read_csv(csv_file, chunksize=10000)
for chunk in chunks:
    process_chunk(chunk)
```
