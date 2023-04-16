import utils
from Plooter import FixPlot, FixPlot_
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1 import make_axes_locatable
import numpy as np
import pandas as pd

cmd_parser = utils.CMDArgs("./output/end.bin", "./imgs/end_lattice.png")

rows, cols, lattice, data = utils.ReadLatticeBinary(cmd_parser.INPUT_FILE)
lattice = lattice / 1.0e-9

mx, my, mz = utils.GetFrameFromBinary(rows, cols, 1, data, 0, 16)
x, y, x_in, y_in, fac_x, fac_y = utils.GetPosition(rows, cols, cmd_parser.REDUCE_FACTOR, lattice)

min_x = -lattice / 2
min_y = -lattice / 2
max_x = (cols - 1) * lattice + lattice / 2
max_y = (rows - 1) * lattice + lattice / 2

if cmd_parser.USE_LATEX: FixPlot(cmd_parser.WIDTH, cmd_parser.HEIGHT)
else: FixPlot_(cmd_parser.WIDTH, cmd_parser.HEIGHT)

r = cols / rows
fig = plt.figure()

if r >= 1:
    fig.set_size_inches(cmd_parser.WIDTH * r, cmd_parser.HEIGHT * 0.75 / 0.83)
    ax = fig.add_axes([0.12, 0.12, 0.75, 0.83])
else:
    fig.set_size_inches(cmd_parser.WIDTH, cmd_parser.HEIGHT * 0.75 / 0.83 / r)
    ax = fig.add_axes([0.12, 0.12, 0.75, 0.83])

img = ax.imshow(mz.reshape([rows, cols]), cmap=utils.cmap, extent=[min_x, max_x, min_y, max_y], origin="lower", vmin=-1, vmax=1, interpolation=cmd_parser.INTERPOLATION)

divider = make_axes_locatable(ax)
cax1 = divider.append_axes("right", size="5%", pad=0.05)
bar = plt.colorbar(img, cax=cax1)
bar.set_ticks([-1, 0, 1])

if cmd_parser.PLOT_ARROWS:
    mx_, my_ = utils.GetVecsFromXY(mx, my, x_in, y_in)
    vecs = ax.quiver(x, y, mx_ * fac_x, my_ * fac_y,
                     angles='xy', scale_units='xy', pivot="mid", scale=1.0, width=0.0013, headwidth=3)

ax.set_xlim([min_x, max_x])
ax.set_ylim([min_y, max_y])

if cmd_parser.USE_LATEX:
    ax.set_xlabel("$x$(nm)")
    ax.set_ylabel("$y$(nm)")
    bar.set_label("$m_z$")
else:
    ax.set_xlabel("x(nm)")
    ax.set_ylabel("y(nm)")
    bar.set_label("m$\\mathsf{_z}$")

# radius in data coordinates:
r = lattice

# radius in display coordinates:
r_ = ax.transData.transform([r,0])[0] - ax.transData.transform([0,0])[0]
r_ = r_ * 72 / fig.dpi

# marker size as the area of a circle
marker_size = r_ * r_

if cmd_parser.PLOT_ANI:
    try:
        ani = pd.read_table(cmd_parser.ANI_INPUT, header=None, sep=cmd_parser.ANI_SEP, skiprows=cmd_parser.ANI_SKIP)
        y_ani = ani[0].to_numpy() * lattice
        x_ani = ani[1].to_numpy() * lattice
    except Exception as e:
        y_ani = np.array([]) * lattice
        x_ani = np.array([]) * lattice
        print(e)
    ax.scatter(x_ani, y_ani, marker="s", s=marker_size, c=cmd_parser.COLOR_ANI)

if cmd_parser.PLOT_PIN:
    try:
        pin = pd.read_table(cmd_parser.PIN_INPUT, header=None, sep=cmd_parser.PIN_SEP, skiprows=cmd_parser.PIN_SKIP)
        y_pin = pin[0].to_numpy() * lattice
        x_pin = pin[1].to_numpy() * lattice
    except Exception as e:
        y_pin = np.array([]) * lattice
        x_pin = np.array([]) * lattice
        print(e)
    ax.scatter(x_pin, y_pin, marker="s", s=marker_size, c=cmd_parser.COLOR_PIN)

cmd_parser.print()
fig.savefig(cmd_parser.OUTPUT_FILE, dpi=cmd_parser.DPI, facecolor="white", bbox_inches="tight")
