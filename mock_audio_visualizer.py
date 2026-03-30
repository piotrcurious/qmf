
import csv
import math

def generate_svg_analysis(csv_file, svg_file, title, width=800, height=400):
    times = []
    rms_l = []
    rms_r = []
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            times.append(float(row['time']))
            rms_l.append(float(row['rms_l']))
            rms_r.append(float(row['rms_r']))

    padding = 50
    max_rms = max(max(rms_l), max(rms_r))

    def x_map(t):
        return padding + (t / max(times)) * (width - 2 * padding)

    def y_map(m):
        return height - padding - (m / max_rms) * (height - 2 * padding)

    with open(svg_file, 'w') as f:
        f.write(f'<svg width="{width}" height="{height}" xmlns="http://www.w3.org/2000/svg" font-family="Arial, sans-serif">\n')
        f.write('<rect width="100%" height="100%" fill="white"/>\n')
        f.write(f'<text x="{width/2}" y="30" text-anchor="middle" font-weight="bold" font-size="16">{title}</text>\n')

        # Axes
        f.write(f'<line x1="{padding}" y1="{height-padding}" x2="{width-padding}" y2="{height-padding}" stroke="black" stroke-width="2"/>\n')
        f.write(f'<line x1="{padding}" y1="{padding}" x2="{padding}" y2="{height-padding}" stroke="black" stroke-width="2"/>\n')

        # Grid lines (time)
        for i in range(11):
            t_label = i * max(times) / 10
            x = x_map(t_label)
            f.write(f'<line x1="{x}" y1="{padding}" x2="{x}" y2="{height-padding}" stroke="#eee" stroke-width="1"/>\n')
            f.write(f'<text x="{x}" y="{height-padding+20}" text-anchor="middle" font-size="10">{t_label:.2f}s</text>\n')

        # RMS Curves
        path_l = f'M {x_map(times[0])} {y_map(rms_l[0])}'
        path_r = f'M {x_map(times[0])} {y_map(rms_r[0])}'
        for i in range(1, len(times)):
            path_l += f' L {x_map(times[i])} {y_map(rms_l[i])}'
            path_r += f' L {x_map(times[i])} {y_map(rms_r[i])}'

        f.write(f'<path d="{path_l}" fill="none" stroke="blue" stroke-width="2"/>\n')
        f.write(f'<path d="{path_r}" fill="none" stroke="red" stroke-width="2"/>\n')

        f.write('<text x="600" y="80" fill="blue" font-size="12">Left (Low-pass)</text>\n')
        f.write('<text x="600" y="100" fill="red" font-size="12">Right (High-pass)</text>\n')

        f.write('</svg>')

def generate_svg_spectrogram(csv_file, svg_file, title, width=800, height=400):
    spec = []
    with open(csv_file, 'r') as f:
        for line in f:
            spec.append([float(x) for x in line.split(',')])

    padding = 50
    num_times = len(spec)
    num_freqs = len(spec[0])

    max_val = max(max(row) for row in spec)

    def color_map(val):
        # Hot map
        intensity = int(255 * min(val / max_val, 1.0))
        return f'rgb({intensity}, {intensity//2}, {255-intensity})'

    cell_w = (width - 2 * padding) / num_times
    cell_h = (height - 2 * padding) / num_freqs

    with open(svg_file, 'w') as f:
        f.write(f'<svg width="{width}" height="{height}" xmlns="http://www.w3.org/2000/svg" font-family="Arial, sans-serif">\n')
        f.write('<rect width="100%" height="100%" fill="black"/>\n')
        f.write(f'<text x="{width/2}" y="30" text-anchor="middle" font-weight="bold" font-size="16" fill="white">{title}</text>\n')

        for i in range(num_times):
            for j in range(num_freqs):
                val = spec[i][j]
                if val > 0.01 * max_val: # Filter noise for performance
                    x = padding + i * cell_w
                    y = height - padding - (j + 1) * cell_h
                    f.write(f'<rect x="{x}" y="{y}" width="{cell_w+0.1}" height="{cell_h+0.1}" fill="{color_map(val)}"/>\n')

        # Labels
        f.write(f'<text x="{padding}" y="{height-padding+20}" fill="white" font-size="10">0s</text>\n')
        f.write(f'<text x="{width-padding}" y="{height-padding+20}" text-anchor="end" fill="white" font-size="10">1s</text>\n')
        f.write(f'<text x="{padding-5}" y="{height-padding}" text-anchor="end" fill="white" font-size="10">0Hz</text>\n')
        f.write(f'<text x="{padding-5}" y="{padding}" text-anchor="end" fill="white" font-size="10">22kHz</text>\n')

        f.write('</svg>')

if __name__ == "__main__":
    generate_svg_analysis('sweep_analysis.csv', 'sweep_analysis.svg', 'Frequency Sweep Analysis (RMS Amplitude)')
    generate_svg_analysis('multitone_analysis.csv', 'multitone_analysis.svg', 'Multi-tone Analysis (RMS Amplitude)')
    generate_svg_spectrogram('sweep_spec_l.csv', 'sweep_spectrogram_l.svg', 'Spectrogram: Low-pass (Left)')
    generate_svg_spectrogram('sweep_spec_r.csv', 'sweep_spectrogram_r.svg', 'Spectrogram: High-pass (Right)')
    print("Mock audio visualization complete.")
