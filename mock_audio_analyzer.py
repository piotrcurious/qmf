
import struct
import math
import csv

def analyze_output(filename, fs=44100):
    """Simple magnitude-only analysis for stereo (Low Left, High Right)"""
    with open(filename, 'rb') as f:
        data = f.read()

    # 16-bit stereo (L, R)
    samples = struct.unpack('<' + 'h' * (len(data) // 2), data)
    left = [s / 32768.0 for s in samples[0::2]]
    right = [s / 32768.0 for s in samples[1::2]]

    # We'll compute RMS in windows to show envelope
    window_size = 1024
    num_windows = len(left) // window_size
    analysis = []
    for i in range(num_windows):
        win_l = left[i * window_size : (i + 1) * window_size]
        win_r = right[i * window_size : (i + 1) * window_size]
        rms_l = math.sqrt(sum(s*s for s in win_l) / window_size)
        rms_r = math.sqrt(sum(s*s for s in win_r) / window_size)
        analysis.append({'time': i * window_size / fs, 'rms_l': rms_l, 'rms_r': rms_r})

    return analysis

def save_analysis(analysis, csv_file):
    with open(csv_file, 'w') as f:
        writer = csv.DictWriter(f, fieldnames=['time', 'rms_l', 'rms_r'])
        writer.writeheader()
        writer.writerows(analysis)

if __name__ == "__main__":
    sweep_results = analyze_output('sweep_output.raw')
    save_analysis(sweep_results, 'sweep_analysis.csv')
    multitone_results = analyze_output('multitone_output.raw')
    save_analysis(multitone_results, 'multitone_analysis.csv')
    print("Mock audio analysis complete.")
