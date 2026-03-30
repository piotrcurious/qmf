
import math
import struct

def generate_sweep(filename, duration=1.0, fs=44100, f0=20, f1=20000):
    num_samples = int(duration * fs)
    with open(filename, 'wb') as f:
        for i in range(num_samples):
            t = i / fs
            # Logarithmic sweep
            freq = f0 * (f1 / f0)**(t / duration)
            phase = 2 * math.pi * f0 * duration * ((f1 / f0)**(t / duration) - 1) / math.log(f1 / f0)
            sample = math.sin(phase)
            f.write(struct.pack('d', sample))

def generate_multitone(filename, freqs=[440, 1000, 5000, 15000], duration=1.0, fs=44100):
    num_samples = int(duration * fs)
    with open(filename, 'wb') as f:
        for i in range(num_samples):
            t = i / fs
            sample = sum(math.sin(2 * math.pi * freq * t) for freq in freqs) / len(freqs)
            f.write(struct.pack('d', sample))

if __name__ == "__main__":
    generate_sweep('mock_sweep.raw')
    generate_multitone('mock_multitone.raw')
    print("Mock audio files generated.")
