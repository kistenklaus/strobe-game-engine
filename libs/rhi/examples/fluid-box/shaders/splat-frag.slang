struct FragmentInput
{
    [[vk::location(0)]]
    float2 localPosition;
};

struct FragmentOutput
{
    [[vk::location(0)]]
    float density;
};

[shader("fragment")]
FragmentOutput main(FragmentInput input)
{
    float r2 = dot(input.localPosition, input.localPosition);
    if (r2 >= 1.0)
        discard;
    float x = 1.0 - r2;
    FragmentOutput output;
    output.density = x * x * x;
    return output;
}
