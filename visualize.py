
import csv

def generate_svg_freq_sweep(csv_file, svg_file):
    freqs = []
    low_pass = []
    high_pass = []
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            freqs.append(float(row['freq']))
            low_pass.append(float(row['low_pass']))
            high_pass.append(float(row['high_pass']))

    width = 800
    height = 400
    padding = 50

    max_mag = max(max(low_pass), max(high_pass))

    def x_map(f):
        return padding + (f / 0.5) * (width - 2 * padding)

    def y_map(m):
        return height - padding - (m / max_mag) * (height - 2 * padding)

    with open(svg_file, 'w') as f:
        f.write(f'<svg width="{width}" height="{height}" xmlns="http://www.w3.org/2000/svg">\n')
        f.write('<rect width="100%" height="100%" fill="white"/>\n')

        # Axes
        f.write(f'<line x1="{padding}" y1="{height-padding}" x2="{width-padding}" y2="{height-padding}" stroke="black" stroke-width="2"/>\n')
        f.write(f'<line x1="{padding}" y1="{padding}" x2="{padding}" y2="{height-padding}" stroke="black" stroke-width="2"/>\n')

        # Labels
        f.write(f'<text x="{width/2}" y="{height-10}" text-anchor="middle">Frequency</text>\n')
        f.write(f'<text x="15" y="{height/2}" transform="rotate(-90, 15, {height/2})" text-anchor="middle">Magnitude</text>\n')

        # Low-pass curve
        path_l = f'M {x_map(freqs[0])} {y_map(low_pass[0])}'
        for i in range(1, len(freqs)):
            path_l += f' L {x_map(freqs[i])} {y_map(low_pass[i])}'
        f.write(f'<path d="{path_l}" fill="none" stroke="blue" stroke-width="2"/>\n')

        # High-pass curve
        path_h = f'M {x_map(freqs[0])} {y_map(high_pass[0])}'
        for i in range(1, len(freqs)):
            path_h += f' L {x_map(freqs[i])} {y_map(high_pass[i])}'
        f.write(f'<path d="{path_h}" fill="none" stroke="red" stroke-width="2"/>\n')

        f.write('<text x="600" y="100" fill="blue">Low-pass</text>\n')
        f.write('<text x="600" y="120" fill="red">High-pass</text>\n')

        f.write('</svg>')

def generate_svg_recon(csv_file, svg_file):
    t = []
    original = []
    reconstructed = []
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            t.append(float(row['t']))
            original.append(float(row['original']))
            reconstructed.append(float(row['reconstructed']))

    width = 800
    height = 400
    padding = 50

    max_val = max(max(original), max(reconstructed))
    min_val = min(min(original), min(reconstructed))
    rng = max_val - min_val

    def x_map(ti):
        return padding + (ti / max(t)) * (width - 2 * padding)

    def y_map(v):
        return height - padding - ((v - min_val) / rng) * (height - 2 * padding)

    with open(svg_file, 'w') as f:
        f.write(f'<svg width="{width}" height="{height}" xmlns="http://www.w3.org/2000/svg">\n')
        f.write('<rect width="100%" height="100%" fill="white"/>\n')

        # Axes
        f.write(f'<line x1="{padding}" y1="{height-padding}" x2="{width-padding}" y2="{height-padding}" stroke="black" stroke-width="2"/>\n')
        f.write(f'<line x1="{padding}" y1="{padding}" x2="{padding}" y2="{height-padding}" stroke="black" stroke-width="2"/>\n')

        # Original curve
        path_o = f'M {x_map(t[0])} {y_map(original[0])}'
        for i in range(1, len(t)):
            path_o += f' L {x_map(t[i])} {y_map(original[i])}'
        f.write(f'<path d="{path_o}" fill="none" stroke="green" stroke-width="2" stroke-opacity="0.5"/>\n')

        # Reconstructed curve (shifted to match delay)
        # Assuming delay is N-1
        delay = 8 - 1
        path_r = f'M {x_map(t[delay])} {y_map(reconstructed[delay])}'
        for i in range(delay + 1, len(t)):
            path_r += f' L {x_map(t[i])} {y_map(reconstructed[i])}'
        f.write(f'<path d="{path_r}" fill="none" stroke="purple" stroke-width="2" stroke-dasharray="5,5"/>\n')

        f.write('<text x="600" y="50" fill="green">Original</text>\n')
        f.write('<text x="600" y="70" fill="purple">Reconstructed (dashed)</text>\n')

        f.write('</svg>')

generate_svg_freq_sweep('sweep.csv', 'sweep.svg')
generate_svg_recon('recon.csv', 'recon.svg')
