#include "slide_renderer.hh"
#include <cstdio>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <markdown_file>\n", argv[0]);
        printf("\nExample markdown format:\n");
        printf("# Title Slide\n");
        printf("This is the content\n");
        printf("---\n");
        printf("## Second Slide\n");
        printf("- Bullet point 1\n");
        printf("- Bullet point 2\n");
        printf("---\n");
        printf("### Code Example\n");
        printf("```cpp\n");
        printf("int main() {\n");
        printf("    return 0;\n");
        printf("}\n");
        printf("```\n");
        printf("---\n");
        printf("### Shell Command Demo\n");
        printf("```$ls -la\n");
        printf("```\n");
        printf("```$date\n");
        printf("```\n");
        return 1;
    }

    MarkdownSlideRenderer renderer;
    renderer.load_slides(argv[1]);
    renderer.run();

    return 0;
}
