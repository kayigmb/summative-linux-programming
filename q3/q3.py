import vibration


def main():
    # Sample vibration data
    data = [1.0, 2.0, 3.0, 4.0, 5.0]

    print("Data:", data)
    print("RMS:", vibration.rms(data))
    print("Peak-to-Peak:", vibration.peak_to_peak(data))
    print("Standard Deviation:", vibration.std_dev(data))
    print("Count above 3.0:", vibration.above_threshold(data, 3.0))
    print("Summary:", vibration.summary(data))

    # Edge case: empty list
    empty = []
    print("\nEmpty list test")
    print("RMS:", vibration.rms(empty))
    print("Peak-to-Peak:", vibration.peak_to_peak(empty))
    print("Standard Deviation:", vibration.std_dev(empty))
    print("Count above 0.0:", vibration.above_threshold(empty, 0.0))
    print("Summary:", vibration.summary(empty))

    # Edge case: invalid input
    try:
        vibration.rms([1, "a", 3])
    except TypeError as e:
        print("\nInvalid input test:", e)


if __name__ == "__main__":
    main()
