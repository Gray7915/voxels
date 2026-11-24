#!/usr/bin/env python3

import argparse
import os


def process_obj(input_file, output_file, scale, center):
    vertices = []
    other_lines = []

    with open(input_file, "r") as f:
        lines = f.readlines()

    # Read vertices
    for line in lines:
        if line.startswith("v "):
            parts = line.strip().split()

            x = float(parts[1])
            y = float(parts[2])
            z = float(parts[3])

            vertices.append([x, y, z])
        else:
            other_lines.append(line)

    if not vertices:
        raise RuntimeError("No vertices found")


    # Calculate pivot
    if center:
        min_x = min(v[0] for v in vertices)
        max_x = max(v[0] for v in vertices)

        min_y = min(v[1] for v in vertices)
        max_y = max(v[1] for v in vertices)

        min_z = min(v[2] for v in vertices)
        max_z = max(v[2] for v in vertices)

        pivot = [
            (min_x + max_x) / 2,
            min_y,
            (min_z + max_z) / 2
        ]
    else:
        pivot = [0, 0, 0]


    output = []

    # Rewrite OBJ
    for line in lines:
        if line.startswith("v "):
            parts = line.strip().split()

            x = float(parts[1])
            y = float(parts[2])
            z = float(parts[3])

            x = (x - pivot[0]) * scale
            y = (y - pivot[1]) * scale
            z = (z - pivot[2]) * scale

            rest = " ".join(parts[4:])

            if rest:
                output.append(
                    f"v {x:.6f} {y:.6f} {z:.6f} {rest}\n"
                )
            else:
                output.append(
                    f"v {x:.6f} {y:.6f} {z:.6f}\n"
                )
        else:
            output.append(line)


    with open(output_file, "w") as f:
        f.writelines(output)

    print(f"Converted: {input_file}")
    print(f"Output:    {output_file}")
    print(f"Scale:     {scale}")
    print(f"Centered:  {center}")
    if center:
        print(f"Pivot:     {pivot}")


def main():
    parser = argparse.ArgumentParser(
        description="Scale and optionally center OBJ voxel models"
    )

    parser.add_argument(
        "input",
        help="Input OBJ file"
    )

    parser.add_argument(
        "output",
        help="Output OBJ file"
    )

    parser.add_argument(
        "--scale",
        type=float,
        required=True,
        help="Scale multiplier. Examples: 2,4,8,0.5,0.25"
    )

    parser.add_argument(
        "--center",
        action="store_true",
        help="Move model pivot to world origin"
    )


    args = parser.parse_args()

    process_obj(
        args.input,
        args.output,
        args.scale,
        args.center
    )


if __name__ == "__main__":
    main()