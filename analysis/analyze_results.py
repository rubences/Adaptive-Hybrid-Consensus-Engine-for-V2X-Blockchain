#!/usr/bin/env python3
"""
Analysis script for V2X Blockchain simulation results
Generates plots for:
1. Latency vs Entropy
2. Energy Efficiency
3. Ledger Divergence
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys
import os

def load_data(csv_file):
    """Load simulation data from CSV file"""
    try:
        df = pd.read_csv(csv_file)
        print(f"Loaded {len(df)} records from {csv_file}")
        return df
    except FileNotFoundError:
        print(f"Error: File {csv_file} not found")
        sys.exit(1)
    except Exception as e:
        print(f"Error loading data: {e}")
        sys.exit(1)

def plot_latency_vs_entropy(df, output_dir):
    """
    Plot latency vs informational and spatial entropy
    Shows how consensus latency changes with network entropy
    """
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
    
    # Latency vs Informational Entropy
    ax1.scatter(df['InformationalEntropy'], df['Latency'] * 1000, 
                c=df['Time'], cmap='viridis', alpha=0.6, s=20)
    ax1.axvline(x=0.5, color='r', linestyle='--', label='S_th = 0.5')
    ax1.set_xlabel('Informational Entropy (S)', fontsize=12)
    ax1.set_ylabel('Latency (ms)', fontsize=12)
    ax1.set_title('Latency vs Informational Entropy', fontsize=14)
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # Latency vs Spatial Entropy
    sc = ax2.scatter(df['SpatialEntropy'], df['Latency'] * 1000,
                     c=df['Time'], cmap='viridis', alpha=0.6, s=20)
    ax2.axvline(x=0.6, color='r', linestyle='--', label='H_th = 0.6')
    ax2.set_xlabel('Spatial Entropy (H_spatial)', fontsize=12)
    ax2.set_ylabel('Latency (ms)', fontsize=12)
    ax2.set_title('Latency vs Spatial Entropy', fontsize=14)
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    
    # Add colorbar
    cbar = plt.colorbar(sc, ax=[ax1, ax2])
    cbar.set_label('Time (s)', fontsize=10)
    
    plt.tight_layout()
    output_file = os.path.join(output_dir, 'latency_vs_entropy.png')
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Saved: {output_file}")
    plt.close()

def plot_energy_efficiency(df, output_dir):
    """
    Plot energy consumption over time
    Compares radio energy vs crypto energy
    """
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    
    # Total energy over time
    df['TotalEnergy'] = df['RadioEnergy'] + df['CryptoEnergy']
    time_bins = np.arange(0, df['Time'].max() + 10, 10)
    df['TimeBin'] = pd.cut(df['Time'], bins=time_bins)
    
    energy_by_time = df.groupby('TimeBin')[['RadioEnergy', 'CryptoEnergy', 'TotalEnergy']].mean()
    time_labels = [f"{int(interval.left)}-{int(interval.right)}" for interval in energy_by_time.index]
    
    # Energy components over time
    ax1 = axes[0, 0]
    x = np.arange(len(energy_by_time))
    width = 0.35
    ax1.bar(x - width/2, energy_by_time['RadioEnergy'], width, label='Radio Energy', alpha=0.8)
    ax1.bar(x + width/2, energy_by_time['CryptoEnergy'], width, label='Crypto Energy', alpha=0.8)
    ax1.set_xlabel('Time (s)', fontsize=12)
    ax1.set_ylabel('Energy (J)', fontsize=12)
    ax1.set_title('Average Energy Consumption Over Time', fontsize=14)
    ax1.set_xticks(x)
    ax1.set_xticklabels(time_labels, rotation=45, ha='right')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # Energy by consensus mode
    ax2 = axes[0, 1]
    energy_by_mode = df.groupby('Mode')[['RadioEnergy', 'CryptoEnergy']].mean()
    energy_by_mode.plot(kind='bar', ax=ax2, alpha=0.8)
    ax2.set_xlabel('Consensus Mode', fontsize=12)
    ax2.set_ylabel('Average Energy (J)', fontsize=12)
    ax2.set_title('Energy Consumption by Consensus Mode', fontsize=14)
    ax2.legend(['Radio Energy', 'Crypto Energy'])
    ax2.grid(True, alpha=0.3)
    plt.setp(ax2.xaxis.get_majorticklabels(), rotation=0)
    
    # Cumulative energy consumption
    ax3 = axes[1, 0]
    df_sorted = df.sort_values('Time')
    ax3.plot(df_sorted['Time'], df_sorted['TotalEnergy'].cumsum(), 
             label='Total Energy', linewidth=2)
    ax3.set_xlabel('Time (s)', fontsize=12)
    ax3.set_ylabel('Cumulative Energy (J)', fontsize=12)
    ax3.set_title('Cumulative Energy Consumption', fontsize=14)
    ax3.legend()
    ax3.grid(True, alpha=0.3)
    
    # Energy efficiency (Energy per transaction)
    ax4 = axes[1, 1]
    mode_counts = df.groupby('Mode').size()
    mode_energy = df.groupby('Mode')['TotalEnergy'].sum()
    energy_per_tx = mode_energy / mode_counts
    ax4.bar(energy_per_tx.index, energy_per_tx.values, alpha=0.8, color=['#ff9999', '#66b3ff'])
    ax4.set_xlabel('Consensus Mode', fontsize=12)
    ax4.set_ylabel('Energy per Record (J)', fontsize=12)
    ax4.set_title('Energy Efficiency by Mode', fontsize=14)
    ax4.grid(True, alpha=0.3)
    plt.setp(ax4.xaxis.get_majorticklabels(), rotation=0)
    
    plt.tight_layout()
    output_file = os.path.join(output_dir, 'energy_efficiency.png')
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Saved: {output_file}")
    plt.close()

def plot_entropy_timeline(df, output_dir):
    """
    Plot entropy evolution over time
    Shows how informational and spatial entropy change during simulation
    """
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(12, 10))
    
    # Informational Entropy over time
    ax1.plot(df['Time'], df['InformationalEntropy'], 'b-', alpha=0.7, linewidth=1.5)
    ax1.axhline(y=0.5, color='r', linestyle='--', label='S_th = 0.5')
    ax1.set_xlabel('Time (s)', fontsize=12)
    ax1.set_ylabel('Informational Entropy (S)', fontsize=12)
    ax1.set_title('Informational Entropy Over Time', fontsize=14)
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # Spatial Entropy over time
    ax2.plot(df['Time'], df['SpatialEntropy'], 'g-', alpha=0.7, linewidth=1.5)
    ax2.axhline(y=0.6, color='r', linestyle='--', label='H_th = 0.6')
    ax2.set_xlabel('Time (s)', fontsize=12)
    ax2.set_ylabel('Spatial Entropy (H_spatial)', fontsize=12)
    ax2.set_title('Spatial Entropy Over Time', fontsize=14)
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    
    # Consensus mode over time
    mode_numeric = df['Mode'].map({'LOW_COST': 0, 'HIGH_RIGOR': 1})
    ax3.fill_between(df['Time'], mode_numeric, alpha=0.5, step='mid')
    ax3.set_xlabel('Time (s)', fontsize=12)
    ax3.set_ylabel('Consensus Mode', fontsize=12)
    ax3.set_yticks([0, 1])
    ax3.set_yticklabels(['LOW_COST', 'HIGH_RIGOR'])
    ax3.set_title('Consensus Mode Switching Over Time', fontsize=14)
    ax3.grid(True, alpha=0.3)
    
    plt.tight_layout()
    output_file = os.path.join(output_dir, 'entropy_timeline.png')
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Saved: {output_file}")
    plt.close()

def plot_ledger_divergence(df, output_dir):
    """
    Plot ledger divergence and fork events
    """
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
    
    # Fork events over time
    fork_df = df[df['ForkEvent'] == 1]
    if len(fork_df) > 0:
        ax1.scatter(fork_df['Time'], fork_df['NodeID'], c='red', marker='x', s=100, label='Fork Events')
        ax1.set_xlabel('Time (s)', fontsize=12)
        ax1.set_ylabel('Node ID', fontsize=12)
        ax1.set_title('Fork Events Timeline', fontsize=14)
        ax1.legend()
        ax1.grid(True, alpha=0.3)
    else:
        ax1.text(0.5, 0.5, 'No fork events detected', 
                horizontalalignment='center', verticalalignment='center',
                transform=ax1.transAxes, fontsize=14)
        ax1.set_title('Fork Events Timeline', fontsize=14)
    
    # Fork events by mode
    forks_by_mode = df.groupby('Mode')['ForkEvent'].sum()
    ax2.bar(forks_by_mode.index, forks_by_mode.values, alpha=0.8, color=['#ff9999', '#66b3ff'])
    ax2.set_xlabel('Consensus Mode', fontsize=12)
    ax2.set_ylabel('Number of Fork Events', fontsize=12)
    ax2.set_title('Fork Events by Consensus Mode', fontsize=14)
    ax2.grid(True, alpha=0.3)
    plt.setp(ax2.xaxis.get_majorticklabels(), rotation=0)
    
    plt.tight_layout()
    output_file = os.path.join(output_dir, 'ledger_divergence.png')
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Saved: {output_file}")
    plt.close()

def print_statistics(df):
    """Print summary statistics"""
    print("\n" + "="*60)
    print("SIMULATION STATISTICS")
    print("="*60)
    
    print(f"\nTotal simulation time: {df['Time'].max():.2f} seconds")
    print(f"Total records: {len(df)}")
    print(f"Number of nodes: {df['NodeID'].nunique()}")
    
    print("\n--- Entropy Statistics ---")
    print(f"Informational Entropy - Mean: {df['InformationalEntropy'].mean():.4f}, "
          f"Std: {df['InformationalEntropy'].std():.4f}")
    print(f"Spatial Entropy - Mean: {df['SpatialEntropy'].mean():.4f}, "
          f"Std: {df['SpatialEntropy'].std():.4f}")
    
    print("\n--- Latency Statistics ---")
    print(f"Mean latency: {df['Latency'].mean() * 1000:.2f} ms")
    print(f"Median latency: {df['Latency'].median() * 1000:.2f} ms")
    print(f"95th percentile: {df['Latency'].quantile(0.95) * 1000:.2f} ms")
    
    print("\n--- Energy Statistics ---")
    print(f"Mean radio energy: {df['RadioEnergy'].mean():.6f} J")
    print(f"Mean crypto energy: {df['CryptoEnergy'].mean():.6f} J")
    print(f"Total energy consumed: {df['RadioEnergy'].sum() + df['CryptoEnergy'].sum():.4f} J")
    
    print("\n--- Consensus Mode Distribution ---")
    mode_counts = df['Mode'].value_counts()
    for mode, count in mode_counts.items():
        percentage = (count / len(df)) * 100
        print(f"{mode}: {count} ({percentage:.1f}%)")
    
    print("\n--- Fork Events ---")
    total_forks = df['ForkEvent'].sum()
    print(f"Total fork events: {int(total_forks)}")
    
    print("\n" + "="*60)

def main():
    """Main analysis function"""
    if len(sys.argv) < 2:
        print("Usage: python analyze_results.py <csv_file> [output_dir]")
        print("Example: python analyze_results.py v2x-urban-metrics.csv ./results")
        sys.exit(1)
    
    csv_file = sys.argv[1]
    output_dir = sys.argv[2] if len(sys.argv) > 2 else './analysis_results'
    
    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    # Load data
    df = load_data(csv_file)
    
    # Print statistics
    print_statistics(df)
    
    # Generate plots
    print("\nGenerating plots...")
    plot_latency_vs_entropy(df, output_dir)
    plot_energy_efficiency(df, output_dir)
    plot_entropy_timeline(df, output_dir)
    plot_ledger_divergence(df, output_dir)
    
    print(f"\nAnalysis complete! Results saved to: {output_dir}")

if __name__ == "__main__":
    main()
