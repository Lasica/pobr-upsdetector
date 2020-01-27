#!/usr/bin/env python

import argparse
import os
import subprocess

def main():
    parser = argparse.ArgumentParser(description='Permutate an image.')
    parser.add_argument('infile', help='input file')
    parser.add_argument('--outdir', '-o', help='output directory', default='perms')
    parser.add_argument('--outbase', '-O', help='output filename base', default='perm')
    parser.add_argument('--rotate-steps', '-r', help='rotation steps', type=int, default=8)
    parser.add_argument('--scale-steps', '-s', help='scale steps', type=int, default=2)
    parser.add_argument('--scale-base', '-S', help='scale base', type=float, default=2)

    args = parser.parse_args()

    assert(os.path.isfile(args.infile))
    assert(os.access(args.infile, os.R_OK))
    assert(os.path.isdir(args.outdir))
    assert(os.access(args.outdir, os.W_OK))

    rotate_step = 360 / args.rotate_steps
    rotations = list(range(args.rotate_steps))
    rotations = [r*rotate_step for r in rotations]
    print(rotations)
    
    scalings = [*[args.scale_base**(-s)*100 for s in range(args.scale_steps, 0, -1)], 100, *[args.scale_base**(s+1)*100 for s in range(args.scale_steps)]]
    print(scalings)

    for r in rotations:
        for s in scalings:
            subprocess.call(
                ['convert', args.infile,
                 '+dither',
                 '-colors', '2',
                 '-colorspace', 'gray',
                 '-normalize',
                 '-resize', f'{s}%',
                 '-background', 'black',
                 '-rotate', f'{r}',
                 f'{args.outdir}/{args.outbase}_{r:.0f}r_{s:.0f}s.png'
                 ])

if __name__=='__main__':
    main()
