#!/bin/bash
set -e

echo "Starting automated mock audio test system..."

# 1. Compile audio_demo
make audio_demo

# 2. Generate mock audio
python3 mock_audio_gen.py

# 3. Process mock audio
echo "Processing frequency sweep..."
./audio_demo mock_sweep.raw sweep_output.raw
echo "Processing multi-tone signal..."
./audio_demo mock_multitone.raw multitone_output.raw

# 4. Analyze outputs
python3 mock_audio_analyzer.py

# 5. Visualize results
python3 mock_audio_visualizer.py

echo "Automated mock audio test system complete. Plots generated."
