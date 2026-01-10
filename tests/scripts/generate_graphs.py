#!/usr/bin/env python3
"""
HerkusBus Performance Testing Graph Generator

This script generates performance graphs from test results and system metrics.
Requires: matplotlib, pandas, numpy
"""

import sys
import os
import csv
from pathlib import Path
from datetime import datetime

try:
    import matplotlib.pyplot as plt
    import matplotlib.patches as mpatches
    import pandas as pd
    import numpy as np
except ImportError as e:
    print(f"Error: Required library not installed: {e}")
    print("Please install required libraries:")
    print("  pip install matplotlib pandas numpy")
    sys.exit(1)

class PerformanceGraphGenerator:
    def __init__(self, perf_test_dir):
        self.perf_test_dir = Path(perf_test_dir)
        self.results_file = self.perf_test_dir / "perf_results.csv"
        self.metrics_file = self.perf_test_dir / "system_metrics.csv"
        self.colors = ["#2E86AB", "#A23B72", "#F18F01"]  # Blue, Purple, Orange
        
    def load_results(self):
        """Load performance test results from CSV"""
        if not self.results_file.exists():
            print(f"Warning: Results file not found: {self.results_file}")
            return None
        
        try:
            df = pd.read_csv(self.results_file)
            return df
        except Exception as e:
            print(f"Error loading results: {e}")
            return None
    
    def load_metrics(self):
        """Load system metrics from CSV"""
        if not self.metrics_file.exists():
            print(f"Warning: Metrics file not found: {self.metrics_file}")
            return None
        
        try:
            df = pd.read_csv(self.metrics_file)
            return df
        except Exception as e:
            print(f"Error loading metrics: {e}")
            return None
    
    def generate_timing_graph(self, results_df):
        """Generate graph showing message sending time vs message count"""
        if results_df is None:
            return
        
        print("Generating timing graphs...")
        
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
        
        message_counts = results_df['MessageCount'].tolist()
        
        # Graph 1: Time by Message Count (grouped by size)
        x = np.arange(len(message_counts))
        width = 0.25
        
        for idx, size_col in enumerate(results_df.columns[1:]):
            times = results_df[size_col].tolist()
            ax1.bar(x + (idx - 1) * width, times, width, 
                   label=size_col, color=self.colors[idx])
        
        ax1.set_xlabel('Message Count', fontsize=12, fontweight='bold')
        ax1.set_ylabel('Time (milliseconds)', fontsize=12, fontweight='bold')
        ax1.set_title('Message Publishing Time by Count and Size', fontsize=14, fontweight='bold')
        ax1.set_xticks(x)
        ax1.set_xticklabels(message_counts)
        ax1.legend()
        ax1.grid(axis='y', alpha=0.3)
        
        # Graph 2: Messages per Second
        for idx, size_col in enumerate(results_df.columns[1:]):
            times = results_df[size_col].tolist()
            msg_per_sec = [(count / (time / 1000)) if time > 0 else 0 
                          for count, time in zip(message_counts, times)]
            ax2.plot(message_counts, msg_per_sec, marker='o', linewidth=2.5,
                    markersize=8, label=size_col, color=self.colors[idx])
        
        ax2.set_xlabel('Message Count', fontsize=12, fontweight='bold')
        ax2.set_ylabel('Messages per Second', fontsize=12, fontweight='bold')
        ax2.set_title('Message Throughput by Size', fontsize=14, fontweight='bold')
        ax2.legend()
        ax2.grid(True, alpha=0.3)
        
        plt.tight_layout()
        output_file = self.perf_test_dir / "timing_performance.png"
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"  Saved: {output_file.name}")
        plt.close()
    
    def generate_system_metrics_graph(self, metrics_df):
        """Generate graph showing CPU and memory usage during tests"""
        if metrics_df is None:
            return
        
        print("Generating system metrics graphs...")
        
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8))
        
        # Convert timestamp to seconds
        timestamps = metrics_df['Timestamp(ms)'].tolist()
        timestamps_sec = [t / 1000.0 for t in timestamps]
        
        # CPU Usage
        cpu_usage = metrics_df['CPU_Usage(%)'].tolist()
        ax1.fill_between(timestamps_sec, cpu_usage, alpha=0.3, color=self.colors[0])
        ax1.plot(timestamps_sec, cpu_usage, linewidth=2, color=self.colors[0], marker='o', markersize=2)
        ax1.set_ylabel('CPU Usage (%)', fontsize=12, fontweight='bold')
        ax1.set_title('System CPU Usage During Performance Tests', fontsize=14, fontweight='bold')
        ax1.grid(True, alpha=0.3)
        ax1.set_ylim(bottom=0)
        
        # Memory Usage
        mem_used = metrics_df['Memory_Used(MB)'].tolist()
        mem_avail = metrics_df['Memory_Available(MB)'].tolist()
        
        ax2.fill_between(timestamps_sec, mem_used, alpha=0.4, color=self.colors[1], label='Used')
        ax2.plot(timestamps_sec, mem_used, linewidth=2, color=self.colors[1], marker='o', markersize=2)
        
        ax2.fill_between(timestamps_sec, mem_avail, alpha=0.2, color=self.colors[2], label='Available')
        ax2.plot(timestamps_sec, mem_avail, linewidth=2, color=self.colors[2], marker='s', markersize=2)
        
        ax2.set_xlabel('Time (seconds)', fontsize=12, fontweight='bold')
        ax2.set_ylabel('Memory (MB)', fontsize=12, fontweight='bold')
        ax2.set_title('System Memory Usage During Performance Tests', fontsize=14, fontweight='bold')
        ax2.legend(loc='upper right')
        ax2.grid(True, alpha=0.3)
        
        plt.tight_layout()
        output_file = self.perf_test_dir / "system_metrics.png"
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"  Saved: {output_file.name}")
        plt.close()
    
    def generate_html_report(self, results_df, metrics_df):
        """Generate HTML report with all results and graphs"""
        if results_df is None:
            return
        
        print("Generating HTML report...")
        
        html_content = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>HerkusBus Performance Test Report</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: #333;
            padding: 20px;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 10px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
            padding: 40px;
        }
        
        h1 {
            text-align: center;
            color: #2E86AB;
            margin-bottom: 10px;
            font-size: 32px;
        }
        
        .timestamp {
            text-align: center;
            color: #777;
            font-size: 14px;
            margin-bottom: 30px;
        }
        
        h2 {
            color: #764ba2;
            margin-top: 30px;
            margin-bottom: 15px;
            border-bottom: 2px solid #667eea;
            padding-bottom: 10px;
        }
        
        .section {
            margin-bottom: 40px;
        }
        
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
            background: #f8f9fa;
        }
        
        th {
            background: #2E86AB;
            color: white;
            padding: 12px;
            text-align: left;
            font-weight: 600;
        }
        
        td {
            padding: 10px 12px;
            border-bottom: 1px solid #ddd;
        }
        
        tr:hover {
            background: #f0f0f0;
        }
        
        .graph {
            margin: 30px 0;
            text-align: center;
        }
        
        .graph img {
            max-width: 100%;
            height: auto;
            border-radius: 5px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        
        .summary {
            background: #f0f8ff;
            border-left: 4px solid #2E86AB;
            padding: 15px;
            margin: 15px 0;
            border-radius: 3px;
        }
        
        .metric {
            display: inline-block;
            background: white;
            padding: 20px;
            margin: 10px;
            border-radius: 5px;
            border-left: 4px solid #A23B72;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
            min-width: 200px;
        }
        
        .metric-label {
            font-weight: 600;
            color: #666;
            font-size: 12px;
        }
        
        .metric-value {
            font-size: 24px;
            color: #2E86AB;
            font-weight: bold;
            margin-top: 5px;
        }
        
        .footer {
            text-align: center;
            margin-top: 40px;
            padding-top: 20px;
            border-top: 1px solid #ddd;
            color: #999;
            font-size: 12px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚌 HerkusBus Performance Test Report</h1>
        <div class="timestamp">Generated on: """ + datetime.now().strftime("%Y-%m-%d %H:%M:%S") + """</div>
        
        <div class="section">
            <h2>📊 Test Results Summary</h2>
            <div class="summary">
                <p>This report contains performance metrics for the HerkusBus message queue system.</p>
                <p>Tests were run with varying message counts (1k, 2k, 5k) and payload sizes (small: 100B, medium: 1KB, large: 10KB).</p>
            </div>
            
            <h3 style="color: #666; margin: 20px 0 10px;">Performance Metrics</h3>
        """
        
        # Add metrics
        if results_df is not None:
            for idx, row in results_df.iterrows():
                msg_count = int(row['MessageCount'])
                for col in results_df.columns[1:]:
                    time_ms = float(row[col])
                    msg_per_sec = (msg_count / (time_ms / 1000)) if time_ms > 0 else 0
                    html_content += f"""
            <div class="metric">
                <div class="metric-label">{msg_count} messages ({col})</div>
                <div class="metric-value">{time_ms:.2f} ms</div>
                <div style="font-size: 12px; color: #999; margin-top: 5px;">{msg_per_sec:.0f} msg/sec</div>
            </div>
            """
        
        html_content += """
        </div>
        
        <div class="section">
            <h2>⏱️ Performance Timing Analysis</h2>
            <div class="graph">
                <img src="timing_performance.png" alt="Timing Performance Graph">
            </div>
        </div>
        """
        
        if metrics_df is not None:
            html_content += """
        <div class="section">
            <h2>💻 System Resource Usage</h2>
            <div class="graph">
                <img src="system_metrics.png" alt="System Metrics Graph">
            </div>
        </div>
        """
        
        html_content += """
        <div class="footer">
            HerkusBus Performance Testing Suite<br>
            <a href="https://github.com/yourusername/HerkusBus" style="color: #667eea;">View on GitHub</a>
        </div>
    </div>
</body>
</html>
        """
        
        output_file = self.perf_test_dir / "report.html"
        with open(output_file, 'w') as f:
            f.write(html_content)
        
        print(f"  Saved: {output_file.name}")
    
    def generate_all(self):
        """Generate all graphs and reports"""
        print(f"\nProcessing performance test data from: {self.perf_test_dir}\n")
        
        results_df = self.load_results()
        metrics_df = self.load_metrics()
        
        if results_df is not None:
            self.generate_timing_graph(results_df)
            self.generate_html_report(results_df, metrics_df)
        
        if metrics_df is not None:
            self.generate_system_metrics_graph(metrics_df)
        
        # Copy graphs to docs/performance for GitHub visibility
        docs_dir = Path(self.perf_test_dir).parent / "docs" / "performance"
        docs_dir.mkdir(parents=True, exist_ok=True)
        
        for graph_file in self.perf_test_dir.glob("*.png"):
            import shutil
            dest_file = docs_dir / graph_file.name
            shutil.copy2(graph_file, dest_file)
            print(f"  Copied to docs: {dest_file.name}")
        
        print(f"\n✓ All graphs and reports generated successfully!")
        print(f"  View the report: {self.perf_test_dir}/report.html\n")


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 generate_graphs.py <perf_test_directory>")
        sys.exit(1)
    
    perf_test_dir = sys.argv[1]
    
    if not Path(perf_test_dir).exists():
        print(f"Error: Directory not found: {perf_test_dir}")
        sys.exit(1)
    
    generator = PerformanceGraphGenerator(perf_test_dir)
    generator.generate_all()


if __name__ == "__main__":
    main()
