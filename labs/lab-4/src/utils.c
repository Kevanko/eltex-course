int strcmp10(const char first[10], const char second[10]) {
    for (int i = 0; i < 10; i++) {
        if (first[i] != second[i])
            return (unsigned char)first[i] - (unsigned char)second[i];
    }
    return 0;
}