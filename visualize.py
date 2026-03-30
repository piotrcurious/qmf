
import csv
import math

def generate_svg_freq_response(csv_file, svg_file, width=800, height=400):
    freqs = []
    low_mag = []
    high_mag = []
    low_phase = []
    high_phase = []
    total_mag = []
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            freqs.append(float(row['freq']))
            low_mag.append(float(row['low_mag']))
            high_mag.append(float(row['high_mag']))
            low_phase.append(float(row['low_phase']))
            high_phase.append(float(row['high_phase']))
            total_mag.append(float(row['total_mag']))

    padding = 50

    max_mag = 2.0 # Standard normalize to sqrt(2) or 2.0

    def x_map(f):
        return padding + (f / 0.5) * (width - 2 * padding)

    def y_map_mag(m):
        return height - padding - (m / max_mag) * (height - 2 * padding)

    def y_map_phase(p):
        return height/2 - (p / math.pi) * (height/2 - padding)

    with open(svg_file, 'w') as f:
        f.write(f'<svg width="{width}" height="{height*2}" xmlns="http://www.w3.org/2000/svg" font-family="Arial, sans-serif">\n')
        f.write('<rect width="100%" height="100%" fill="#ffffff"/>\n')

        # Background Grid lines
        for i in range(11):
            val = i * 0.1
            f.write(f'<line x1="{x_map(val/2)}" y1="{padding}" x2="{x_map(val/2)}" y2="{height-padding}" stroke="#eeeeee" stroke-width="1"/>\n')

        # Magnitude Graph
        f.write(f'<text x="{width/2}" y="30" text-anchor="middle" font-weight="bold" font-size="16">Frequency Response (Magnitude)</text>\n')
        # Axes
        f.write(f'<line x1="{padding}" y1="{height-padding}" x2="{width-padding}" y2="{height-padding}" stroke="black" stroke-width="2"/>\n')
        f.write(f'<line x1="{padding}" y1="{padding}" x2="{padding}" y2="{height-padding}" stroke="black" stroke-width="2"/>\n')

        # Grid
        for i in range(6):
            f.write(f'<text x="{padding + i * (width-2*padding)/5}" y="{height-padding+20}" text-anchor="middle">{i*0.1:.1f}</text>\n')
            f.write(f'<line x1="{padding + i * (width-2*padding)/5}" y1="{padding}" x2="{padding + i * (width-2*padding)/5}" y2="{height-padding}" stroke="#ccc" stroke-dasharray="2,2"/>\n')

        # Magnitude Curves
        path_l = f'M {x_map(freqs[0])} {y_map_mag(low_mag[0])}'
        path_h = f'M {x_map(freqs[0])} {y_map_mag(high_mag[0])}'
        path_t = f'M {x_map(freqs[0])} {y_map_mag(total_mag[0])}'
        for i in range(1, len(freqs)):
            path_l += f' L {x_map(freqs[i])} {y_map_mag(low_mag[i])}'
            path_h += f' L {x_map(freqs[i])} {y_map_mag(high_mag[i])}'
            path_t += f' L {x_map(freqs[i])} {y_map_mag(total_mag[i])}'

        f.write(f'<path d="{path_l}" fill="none" stroke="blue" stroke-width="2"/>\n')
        f.write(f'<path d="{path_h}" fill="none" stroke="red" stroke-width="2"/>\n')
        f.write(f'<path d="{path_t}" fill="none" stroke="green" stroke-width="1" stroke-dasharray="4,4"/>\n')

        f.write('<text x="600" y="80" fill="blue">Low-pass</text>\n')
        f.write('<text x="600" y="100" fill="red">High-pass</text>\n')
        f.write('<text x="600" y="120" fill="green">Total Power (Ideal flat)</text>\n')

        # Phase Graph
        f.write(f'<g transform="translate(0, {height})">\n')
        f.write(f'<text x="{width/2}" y="30" text-anchor="middle" font-weight="bold">Phase Response (Radians)</text>\n')
        f.write(f'<line x1="{padding}" y1="{height-padding}" x2="{width-padding}" y2="{height-padding}" stroke="black" stroke-width="2"/>\n')
        f.write(f'<line x1="{padding}" y1="{padding}" x2="{padding}" y2="{height-padding}" stroke="black" stroke-width="2"/>\n')
        f.write(f'<line x1="{padding}" y1="{height/2}" x2="{width-padding}" y2="{height/2}" stroke="#ccc" stroke-dasharray="5,5"/>\n') # Zero phase line

        path_pl = f'M {x_map(freqs[0])} {y_map_phase(low_phase[0])}'
        path_ph = f'M {x_map(freqs[0])} {y_map_phase(high_phase[0])}'
        for i in range(1, len(freqs)):
            if abs(low_phase[i] - low_phase[i-1]) < 1.0: # Prevent jump lines on wrap
                path_pl += f' L {x_map(freqs[i])} {y_map_phase(low_phase[i])}'
            else:
                path_pl += f' M {x_map(freqs[i])} {y_map_phase(low_phase[i])}'
            if abs(high_phase[i] - high_phase[i-1]) < 1.0:
                path_ph += f' L {x_map(freqs[i])} {y_map_phase(high_phase[i])}'
            else:
                path_ph += f' M {x_map(freqs[i])} {y_map_phase(high_phase[i])}'

        f.write(f'<path d="{path_pl}" fill="none" stroke="blue" stroke-width="2"/>\n')
        f.write(f'<path d="{path_ph}" fill="none" stroke="red" stroke-width="2"/>\n')
        f.write('</g>\n')

        f.write('</svg>')

def generate_svg_recon(csv_file, svg_file, width=800, height=400):
    t = []
    original = []
    reconstructed = []
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            t.append(float(row['t']))
            original.append(float(row['original']))
            reconstructed.append(float(row['reconstructed']))

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
        f.write(f'<text x="{width/2}" y="30" text-anchor="middle" font-weight="bold">Signal Reconstruction (Time Domain)</text>\n')

        # Axes
        f.write(f'<line x1="{padding}" y1="{height-padding}" x2="{width-padding}" y2="{height-padding}" stroke="black" stroke-width="2"/>\n')
        f.write(f'<line x1="{padding}" y1="{padding}" x2="{padding}" y2="{height-padding}" stroke="black" stroke-width="2"/>\n')

        # Original curve
        path_o = f'M {x_map(t[0])} {y_map(original[0])}'
        for i in range(1, len(t)):
            path_o += f' L {x_map(t[i])} {y_map(original[i])}'
        f.write(f'<path d="{path_o}" fill="none" stroke="green" stroke-width="2" stroke-opacity="0.3"/>\n')

        # Reconstructed curve (shifted to match delay)
        # Assuming delay is N-1
        delay = 8 - 1
        path_r = f'M {x_map(t[delay])} {y_map(reconstructed[delay])}'
        for i in range(delay + 1, len(t)):
            path_r += f' L {x_map(t[i])} {y_map(reconstructed[i])}'
        f.write(f'<path d="{path_r}" fill="none" stroke="purple" stroke-width="2" stroke-dasharray="3,3"/>\n')

        f.write('<text x="600" y="50" fill="green">Original</text>\n')
        f.write('<text x="600" y="70" fill="purple">Reconstructed (dashed, delay corrected)</text>\n')

        f.write('</svg>')

generate_svg_freq_response('sweep.csv', 'freq_response.svg')
generate_svg_recon('recon.csv', 'reconstruction.svg')
