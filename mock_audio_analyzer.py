
import struct
import math
import csv

def fft(x):
    n = len(x)
    if n <= 1: return x
    even = fft(x[0::2])
    odd =  fft(x[1::2])
    T = [math.e**(-2j * math.pi * k / n) * odd[k] for k in range(n // 2)]
    return [even[k] + T[k] for k in range(n // 2)] + \
           [even[k] - T[k] for k in range(n // 2)]

def analyze_output(filename, fs=44100):
    """Magnitude and Spectrogram analysis for stereo (Low Left, High Right)"""
    with open(filename, 'rb') as f:
        data = f.read()

    samples = struct.unpack('<' + 'h' * (len(data) // 2), data)
    left = [s / 32768.0 for s in samples[0::2]]
    right = [s / 32768.0 for s in samples[1::2]]

    window_size = 512 # Power of 2 for FFT
    num_windows = len(left) // window_size
    analysis = []
    spectrogram_l = []
    spectrogram_r = []

    for i in range(num_windows):
        win_l = left[i * window_size : (i + 1) * window_size]
        win_r = right[i * window_size : (i + 1) * window_size]

        # RMS
        rms_l = math.sqrt(sum(s*s for s in win_l) / window_size)
        rms_r = math.sqrt(sum(s*s for s in win_r) / window_size)

        # FFT (complex)
        fft_l = fft([complex(s, 0) for s in win_l])
        fft_r = fft([complex(s, 0) for s in win_r])

        # Magnitude (first half)
        mag_l = [abs(c) for c in fft_l[:window_size // 2]]
        mag_r = [abs(c) for c in fft_r[:window_size // 2]]

        analysis.append({'time': i * window_size / fs, 'rms_l': rms_l, 'rms_r': rms_r})
        spectrogram_l.append(mag_l)
        spectrogram_r.append(mag_r)

    return analysis, spectrogram_l, spectrogram_r

def save_analysis(analysis, csv_file):
    with open(csv_file, 'w') as f:
        writer = csv.DictWriter(f, fieldnames=['time', 'rms_l', 'rms_r'])
        writer.writeheader()
        writer.writerows(analysis)

def save_spectrogram(spec, filename):
    with open(filename, 'w') as f:
        for row in spec:
            f.write(','.join(map(str, row)) + '\n')

if __name__ == "__main__":
    for tag in ['sweep', 'multitone']:
        analysis, spec_l, spec_r = analyze_output(f'{tag}_output.raw')
        save_analysis(analysis, f'{tag}_analysis.csv')
        save_spectrogram(spec_l, f'{tag}_spec_l.csv')
        save_spectrogram(spec_r, f'{tag}_spec_r.csv')
    print("Mock audio spectral analysis complete.")
