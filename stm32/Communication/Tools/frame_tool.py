#!/usr/bin/env python3
"""Generate or decode the assignment's UART frames without extra packages.

Examples:
    python Tools/frame_tool.py brightness 5
    python Tools/frame_tool.py voltage 2048
    python Tools/frame_tool.py decode "AA 55 05 02 00 08 72 06 82"
    python Tools/frame_tool.py selftest
"""

from __future__ import annotations

import argparse
import sys


def crc8(data: bytes) -> int:
    crc = 0
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x80:
                crc = ((crc << 1) ^ 0x07) & 0xFF
            else:
                crc = (crc << 1) & 0xFF
    return crc


def build_frame(payload: bytes) -> bytes:
    if not 1 <= len(payload) <= 8:
        raise ValueError("payload length must be 1..8")
    body = bytes([len(payload)]) + payload
    return bytes([0xAA, 0x55]) + body + bytes([crc8(body)])


def parse_stream(data: bytes):
    """Yield frames as (payload, raw_frame) and skip garbage/bad CRC."""
    i = 0
    while i < len(data):
        if data[i] != 0xAA or i + 1 >= len(data):
            i += 1
            continue
        if data[i + 1] != 0x55:
            i += 1
            continue
        if i + 2 >= len(data):
            break
        length = data[i + 2]
        if not 1 <= length <= 8:
            i += 1
            continue
        total = 3 + length + 1
        if i + total > len(data):
            break
        payload = data[i + 3 : i + 3 + length]
        if crc8(bytes([length]) + payload) != data[i + 3 + length]:
            i += 1
            continue
        yield payload, data[i : i + total]
        i += total


def adc_to_mv(adc: int) -> int:
    return (adc * 3300 + 2047) // 4095


def hex_bytes(text: str) -> bytes:
    cleaned = text.replace(",", " ").replace("0x", " ").replace("0X", " ")
    return bytes.fromhex(cleaned)


def format_hex(data: bytes) -> str:
    return " ".join(f"{byte:02X}" for byte in data)


def cmd_brightness(args: argparse.Namespace) -> int:
    if not 0 <= args.level <= 10:
        raise ValueError("LEVEL must be 0..10")
    frame = build_frame(bytes([0x01, args.level]))
    print(format_hex(frame))
    return 0


def cmd_voltage(args: argparse.Namespace) -> int:
    if not 0 <= args.adc <= 4095:
        raise ValueError("ADC must be 0..4095")
    mv = adc_to_mv(args.adc) if args.mv is None else args.mv
    if not 0 <= mv <= 0xFFFF:
        raise ValueError("MV must be 0..65535")
    payload = bytes(
        [0x02, args.adc & 0xFF, (args.adc >> 8) & 0xFF, mv & 0xFF, (mv >> 8) & 0xFF]
    )
    print(format_hex(build_frame(payload)))
    return 0


def cmd_decode(args: argparse.Namespace) -> int:
    data = hex_bytes(args.hex)
    frames = list(parse_stream(data))
    if not frames:
        print("No valid frame found.", file=sys.stderr)
        return 1

    for index, (payload, raw) in enumerate(frames, start=1):
        print(f"frame {index}: {format_hex(raw)}")
        if len(payload) == 2 and payload[0] == 0x01:
            print(f"  brightness: level={payload[1]}")
        elif len(payload) == 5 and payload[0] == 0x02:
            adc = payload[1] | (payload[2] << 8)
            mv = payload[3] | (payload[4] << 8)
            print(f"  voltage: ADC={adc}, MV={mv}, approx={mv / 1000.0:.3f} V")
        else:
            print(f"  payload: {format_hex(payload)} (unknown format)")
    return 0


def cmd_selftest(_: argparse.Namespace) -> int:
    cases = [
        (bytes([0x01, 0x00]), "AA 55 02 01 00 C3"),
        (bytes([0x01, 0x05]), "AA 55 02 01 05 D8"),
        (bytes([0x01, 0x0A]), "AA 55 02 01 0A F5"),
        (
            bytes([0x02, 0x00, 0x08, 0x72, 0x06]),
            "AA 55 05 02 00 08 72 06 82",
        ),
    ]
    for payload, expected in cases:
        actual = format_hex(build_frame(payload))
        if actual != expected:
            print(f"FAIL: expected {expected}, got {actual}", file=sys.stderr)
            return 1
    print("PASS: all frame examples match.")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)

    p_brightness = sub.add_parser("brightness", help="build a PC -> board A command")
    p_brightness.add_argument("level", type=int, help="0..10")
    p_brightness.set_defaults(func=cmd_brightness)

    p_voltage = sub.add_parser("voltage", help="build a board A -> PC voltage frame")
    p_voltage.add_argument("adc", type=int, help="0..4095")
    p_voltage.add_argument("mv", nargs="?", type=int, help="optional millivolts")
    p_voltage.set_defaults(func=cmd_voltage)

    p_decode = sub.add_parser("decode", help="decode hex bytes from VOFA+ or a log")
    p_decode.add_argument("hex", help="e.g. 'AA 55 05 02 00 08 72 06 82'")
    p_decode.set_defaults(func=cmd_decode)

    p_test = sub.add_parser("selftest", help="verify the documented examples")
    p_test.set_defaults(func=cmd_selftest)

    args = parser.parse_args()
    try:
        return args.func(args)
    except ValueError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())