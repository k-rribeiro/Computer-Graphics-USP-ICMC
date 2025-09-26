//bibliotecas----------------------------------------
#include <iostream>
#include <vector>
#include <algorithm>
#include <GL/glut.h>
#include <cmath>
#include <string>
#include <iomanip>

using namespace std;

//struct para armazenar os valores de x e y
struct Ponto {
    int x, y;
};

//struct da aresta que guarda os valores importantes para fazer o calculo
struct Aresta {
    int ymax;     
    double x_atual; 
    double m;     

    Aresta(int y_max, double x_atual, double m) 
        : ymax(y_max), x_atual(x_atual), m(m) {}

};


//tabela ET - Edge Table
typedef std::vector<std::vector<Aresta>> ET;


//funcao para a criação da ET
ET construirET(const std::vector<Ponto>& vertices) { 
    const int MAX_Y = 500; // Defina o tamanho máximo do seu 'canvas' (ET)
    ET edge_table(MAX_Y);
    
    //caso tenha menos que dois pontos nao é um poligono
    if (vertices.size() < 2) {
        std::cerr << "O vetor deve conter pelo menos 2 pontos para formar arestas." << std::endl;
    }

    //itera sobre todas as arestas
    for (size_t i = 0; i < vertices.size(); ++i) {
        
        //define o par de pontos, tipo o valor que escreveu na primeira posicao e na segunda posição do vetor irão compor o ponto1
        Ponto p1 = vertices[i];
        Ponto p2 = vertices[(i + 1) % vertices.size()]; // Usa módulo para fechar o polígono


        //tratamento para arestas horizontais (y = 0)
        if (p1.y == p2.y) {
            continue; // Pula para a próxima aresta
        }

        //ordena os pontos da aresta
        Ponto min = p1;
        Ponto max = p2;

        if (p1.y > p2.y) {
            //caso um valor é maior que o outro troca para fazer a conta
            std::swap(min, max);
        }

        //m = dy/dx
        double Dy = static_cast<double>(max.y - min.y);
        double Dx = static_cast<double>(max.x - min.x);
        double M = Dy / Dx; 

        double atualx = static_cast<double>(min.x);

        
        //loop de varredura: x_{i+1} = x_i + 1/m
        for (int y = min.y; y <= max.y; y++) {
            //e incrementa x 
            atualx += M;

        }
    }

return edge_table;
}



void Inicio( const std::vector<Ponto>& vertices) {
    //AET
    std::vector<Aresta> AET;

    //monta a ET
    ET et = construirET(vertices);

    //separa a menor coordenada y 
    int y_scan = 0;
    while (y_scan < et.size() && et[y_scan].empty()) {
        y_scan++;
    }

    
    //vai repetindo esse processo até que a ET e a AET estejam vazias
    while (y_scan < et.size() || !AET.empty()) {
        
        //transferindo da ET para a AET 
        if (y_scan < et.size() && !et[y_scan].empty()) {
            //adiciona todas as arestas de y_scan na AET
            AET.insert(AET.end(), et[y_scan].begin(), et[y_scan].end());
            
            //reordena a AET em x 
            std::sort(AET.begin(), AET.end(), [](const Aresta& a, const Aresta& b) {
                return a.x_atual < b.x_atual;
            });
            
        }

        //retira os lados que possuem y = ymax (arestas terminando)
        auto it_remove = std::remove_if(AET.begin(), AET.end(), 
                                        [y_scan](const Aresta& a) {
            return a.ymax == y_scan;
        });
        
        AET.erase(it_remove, AET.end());

        
        //ve se a varredura terminou
        if (AET.empty() && y_scan >= et.size()) {
            break; 
        }


        //próxima linha de varredura
        y_scan++;
        
        //para cada aresta que permanece na AET, atualiza x para o novo y
        for (Aresta& a : AET) {
            a.x_atual += a.m;
            
        }

        //reordena a AET 
        std::sort(AET.begin(), AET.end(), [](const Aresta& a, const Aresta& b) {
            return a.x_atual < b.x_atual;
        });
       
    }

}

//funçao principal--------------------------
int main() {
    int vert;

    std::cout << "Quantos vertices tem o poligono? " << std::endl;
    std::cin >> vert;
    std::cout << "seu poligono tem " << vert << " vertices "<< std::endl;

    //alimentando um vetor com os pontos (x,y) do poligono de acordo com o numero de vertices
    //nao sei alimentar um vetor ainda entao botei numeros aleatorios
    std::vector<Ponto> exemplo = {
            {10, 10}, 
            {20, 30}, 
            {40, 30}, 
            {50, 10}
    };

    Inicio(exemplo);

    return 0;
}