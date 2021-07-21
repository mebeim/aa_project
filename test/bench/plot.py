#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import sys
import re
import json
import math
import numpy as np
from pathlib import Path
from collections import defaultdict
from sklearn.linear_model import LinearRegression
from sklearn.preprocessing import PolynomialFeatures
from matplotlib import pyplot as plt, colors

def parse_benchmark_output_time(fname):
	funcnames = {
		'fill_random_graph'   : 'FILL',
		'fill_in_random_graph': 'FILL',
		'lex_m_random_graph'  : 'LEX M',
		'lex_p_random_graph'  : 'LEX P',
	}

	with open(fname) as f:
		data = json.load(f)

	time = defaultdict(lambda: defaultdict(lambda: ([], [], [])))
	bench_name_exp = re.compile(r'([\w_]+)<(\d+), (\d+)>')

	for bench in data['benchmarks']:
		if bench['name'].endswith('BigO') or bench['name'].endswith('RMS'):
			continue

		func, num, div = bench_name_exp.findall(bench['name'])[0]
		perc = 100 * int(num) // int(div)
		func = funcnames.get(func, func)
		time[func][perc][0].append(bench["n"])
		time[func][perc][1].append(bench["v"])
		time[func][perc][2].append(bench["cpu_time"])

	return time

def parse_benchmark_output_mem(fname):
	funcnames = {
		'fill_random_graph'   : 'FILL',
		'fill_in_random_graph': 'FILL',
		'lex_m_random_graph'  : 'LEX M',
		'lex_p_random_graph'  : 'LEX P',
	}

	with open(fname) as f:
		data = f.read()

	mem = defaultdict(lambda: defaultdict(lambda: ([], [], [])))
	line_exp = re.compile(r'([\w_]+)<(\d+), ?(\d+)> v=(\d+) n=(\d+): max (\d+) bytes')

	for func, *nums in line_exp.findall(data):
		num, div, v, n, max_mem = map(int, nums)
		perc = 100 * num // div
		func = funcnames.get(func, func)
		mem[func][perc][0].append(n)
		mem[func][perc][1].append(v)
		mem[func][perc][2].append(max_mem)

	return mem

def gradient_color(perc):
	return colors.hsv_to_rgb((perc / 100, 1, 1))

def int_to_human(n):
	suff = ['', 'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y']

	if n < 1000:
		return str(int(n))

	order = math.floor(math.log10(n) / 3)

	if order >= len(suff):
		return f'{n / (1000 ** (len(suff) - 1)):.0f}' + suff[-1]
	return f'{n / (1000 ** order):.0f}' + suff[order]

def linear_regression(xs, ys):
	xx = np.array(xs)
	xx = xx[:, np.newaxis]
	lr = LinearRegression()
	lr.fit(xx, ys)
	return lr.predict(xx), lr.coef_[0]

def polynomial_regression(xs, ys, degree):
	xx = np.array(xs)
	xx = xx[:, np.newaxis]
	lr = LinearRegression()
	pf = PolynomialFeatures(degree)
	pf_x = pf.fit_transform(xx, ys)
	lr.fit(pf_x, ys)
	return lr.predict(pf_x)

def plot_time(title, data, out_dir):
	subplot_titles = {
		'FILL' : ('T = f(V) w/ quadratic regression', 'T = f(V + E) w/ linear regression'),
		'LEX P': ('T = f(V) w/ quadratic regression', 'T = f(V + E) w/ linear regression'),
		'LEX M': ('T = f(V) w/ cubic regression', 'T = f(V + E) w/ linear regression'),
	}

	print('Plotting', title, 'time', file=sys.stderr)

	plt.clf()

	fig, axes = plt.subplots(1, 2, dpi=192, figsize=(11.5, 5))
	axes[0].set_ylabel('CPU Time (ms)')
	axes[0].set_title(subplot_titles[title][0])
	axes[1].set_title(subplot_titles[title][1])
	plt.subplots_adjust(wspace=0.15)

	for ax in axes:
		ax.grid(True, linestyle='--')
		ax.set_axisbelow(True)
		ax.xaxis.set_major_formatter(lambda v, _: int_to_human(v))

	# ploynomial plot in V
	ax = axes[0]
	ax.set_xlabel('V')
	ax.xaxis.set_ticks(data[next(iter(data.keys()))][1])

	for perc, (_, xs, ys) in data.items():
		reg_y = polynomial_regression(xs, ys, 3 if title == 'LEX M' else 2)

		ax.scatter(xs, ys, s=12, color=gradient_color(perc))
		ax.plot(xs, reg_y, color=gradient_color(perc), alpha=0.5, label=f'{perc}%')

	# linear plot in VE or V + E
	ax = axes[1]
	ax.set_xlabel('VE' if title == 'LEX M' else 'V + E')

	for perc, (xs, _, ys) in data.items():
		reg_y, coef = linear_regression(xs, ys)
		coef *= 1e6

		ax.scatter(xs, ys, s=12, color=gradient_color(perc))
		ax.plot(xs, reg_y, color=gradient_color(perc), alpha=0.5, label=f'{perc}%')

		xytext = (-30, 5)
		if title == 'FILL':
			if perc != 100:
				xytext = (5, 5) if perc == 66 else (5, 0)
		elif title == 'LEX P' and perc == 75:
			xytext = (-30, -10)

		ax.annotate(f'm={coef:.2f}', (xs[-1], reg_y[-1]), xytext=xytext, textcoords='offset points', fontsize=8)

	handles, labels = ax.get_legend_handles_labels()
	fig.legend(handles, labels, loc='center right', title='edge density', bbox_to_anchor=(0.001,0,1,1), bbox_transform=fig.transFigure)
	plt.savefig(Path(out_dir) / f'time_{title.lower().replace(" ", "_")}.png')

def plot_mem(title, data, out_dir):
	subplot_titles = {
		'FILL' : ('S = f(V) w/ quadratic regression', 'S = f(V + E) w/ linear regression'),
		'LEX P': ('S = f(V) w/ quadratic regression', 'S = f(V + E) w/ linear regression'),
		'LEX M': 'S = f(V) w/ linear regression',
	}

	print('Plotting', title, 'mem', file=sys.stderr)

	plt.clf()

	if title == 'LEX M':
		# linear plot in V only
		fig, ax = plt.subplots(1, 1, dpi=192, figsize=(10, 6))

		ax.set_title(subplot_titles[title])
		ax.set_ylabel('Allocated memory (bytes)')
		ax.grid(True, linestyle='--')
		ax.set_axisbelow(True)
		ax.xaxis.set_major_formatter(lambda v, _: int_to_human(v))
		ax.yaxis.set_major_formatter(lambda v, _: int_to_human(v))
		ax.set_xlim(50, 1100)
		ax.set_xlabel('V')
		ax.xaxis.set_ticks(data[next(iter(data.keys()))][1])

		for perc, (_, xs, ys) in data.items():
			reg_y, coef = linear_regression(xs, ys)

			ax.scatter(xs, ys, s=12, color=gradient_color(perc))
			ax.plot(xs, reg_y, color=gradient_color(perc), alpha=0.5, label=f'{perc}%')
			ax.annotate(f'm={coef:.2f}', (xs[-1], reg_y[-1]), xytext=(5, 0), textcoords='offset points', fontsize=8)

		ax.legend(title='edge density')
	else:
		fig, axes = plt.subplots(1, 2, dpi=192, figsize=(11.5, 5))
		plt.subplots_adjust(wspace=0.15)
		axes[0].set_ylabel('Allocated memory (bytes)')
		axes[0].set_title(subplot_titles[title][0])
		axes[1].set_title(subplot_titles[title][1])

		for ax in axes:
			ax.grid(True, linestyle='--')
			ax.set_axisbelow(True)
			ax.xaxis.set_major_formatter(lambda v, _: int_to_human(v))
			ax.yaxis.set_major_formatter(lambda v, _: int_to_human(v))

		# ploynomial plot in V
		ax = axes[0]
		ax.set_xlabel('V')
		ax.xaxis.set_ticks(data[next(iter(data.keys()))][1])

		for perc, (_, xs, ys) in data.items():
			reg_y = polynomial_regression(xs, ys, 2)

			ax.scatter(xs, ys, s=12, color=gradient_color(perc))
			ax.plot(xs, reg_y, color=gradient_color(perc), alpha=0.5, label=f'{perc}%')

		# linear plot in V + E
		ax = axes[1]
		ax.set_xlabel('V + E')

		for perc, (xs, _, ys) in data.items():
			reg_y, coef = linear_regression(xs, ys)

			ax.scatter(xs, ys, s=12, color=gradient_color(perc))
			ax.plot(xs, reg_y, color=gradient_color(perc), alpha=0.5, label=f'{perc}%')

			xytext = (-35, 5) if (perc == 100 or (title == 'LEX P' and perc == 66)) else (5, 0)
			ax.annotate(f'm={coef:.2f}', (xs[-1], reg_y[-1]), xytext=xytext, textcoords='offset points', fontsize=8)

		handles, labels = ax.get_legend_handles_labels()
		fig.legend(handles, labels, loc='center right', title='edge density', bbox_to_anchor=(0.001,0,1,1), bbox_transform=fig.transFigure)

	plt.savefig(Path(out_dir) / f'mem_{title.lower().replace(" ", "_")}.png')

################################################################################

if __name__ == '__main__':
	if len(sys.argv) != 4:
		print(f'Usage: {sys.argv[0]} BENCH_TIME_OUT.json BENCH_MEM_OUT.txt OUT_DIR', file=sys.stderr)
		sys.exit(1)

	time_data = parse_benchmark_output_time(sys.argv[1])
	mem_data =  parse_benchmark_output_mem(sys.argv[2])

	for func, data in time_data.items():
		plot_time(func, data, sys.argv[3])

	for func, data in mem_data.items():
		plot_mem(func, data, sys.argv[3])
