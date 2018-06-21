vec4 convertToGray(vec4 color) {
    float gray = (color.r + color.g + color.b) / 3.0;
    return vec4(gray, gray, gray, 1.0);
}

vec4 conv3x3(sampler2D inputTexture, float width, float height, vec2 textureCoord, mat3x3 kernel) {
    vec4 color;

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            color += kernel[x + 1][y + 1] * texture(inputTexture, textureCoord + vec2(x / width, y / height));
        }
    }

    return color;
}