#include <stdio.h>

int main()
{
    float altura, largura, profundidade;
    float area_superficie, volume;

    printf("Digite a altura da caixa:\n");
    scanf("%f", &altura);

    printf("Digite a profundidade da caixa:\n");
    scanf("%f", &profundidade);

    printf("Digite a largura da caixa:\n");
    scanf("%f", &largura);

    area_superficie = 2 * (altura * largura + altura * profundidade + largura * profundidade);

    return (0);
}
