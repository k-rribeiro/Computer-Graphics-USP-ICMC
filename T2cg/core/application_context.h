//essa classe serve como o um local central para gerenciar 
//o estado, os modulos e a interaçao do aplicativo

//includes e dependencias
#ifndef APPLICATION_CONTEXT_H
#define APPLICATION_CONTEXT_H

#include "data_structures.h"
#include "polygon_manager.h"
#include "graphics_renderer.h"
#include "scene_manager.h"
#include "event_handler.h"
#include "ui_manager.h"
#include <GL/glut.h>

//nesta parte definimos os modos de operação
enum class AppMode {
    //aqui o usuario desenha e edita os poligonos 2d
    MODE_2D_EDITOR,
    //aqui os poligonos desenhados se tornam objetos 3D e podem ser visualizados
    MODE_3D_VIEWER
};

//definicao de instancias e armazenamento
class ApplicationContext {
public:
    PolygonManager polygonManager;
    GraphicsRenderer graphicsRenderer;
    SceneManager sceneManager;
    EventHandler* eventHandler;
    UIManager uiManager;
    
    WindowDimensions* windowDimensions;
    ApplicationState applicationState;
    AppMode currentMode;
    
    int lastMouseX;
    int lastMouseY;
    bool isRightMouseButtonPressed;

    //aqui inicializa os membros de dados com valores padrao
    ApplicationContext() 
        : eventHandler(nullptr), windowDimensions(nullptr),
          applicationState(ApplicationState::DRAWING_POLYGON),
          currentMode(AppMode::MODE_2D_EDITOR),
          lastMouseX(0), lastMouseY(0), isRightMouseButtonPressed(false) {
    }

    //limpa a memoria
    ~ApplicationContext() {
        if (eventHandler) delete eventHandler;
        if (windowDimensions) delete windowDimensions;
    }

    //este metodo tem a funcao de configurar o ambiente e os subsistemas
    void init() {
        //definimos a cor de fundo do opengl
        glClearColor(0.12f, 0.12f, 0.18f, 1.0f); // Dark background (#1e1e2e)
        //inicia o membro que gerencia objetos 3D, câmeras, iluminação e as projeções
        sceneManager.init();
        
        windowDimensions = new WindowDimensions(WINDOW_WIDTH, WINDOW_HEIGHT);
        
        //inicializa UI Manager
        uiManager.init(&currentMode);
        
        //ocorre a definicao dos callbacks para a UI controlar as cenas 3D, onde aciona uma 
        //função que executa a lógica no contexto
        uiManager.setCallbacks(
            //controle de shading (FLAT, GOURAUD, PHONG)
            [this](ShadingMode mode) {
                switch (mode) {
                    case ShadingMode::FLAT:
                        sceneManager.setLightingModel(LightingModel::FLAT);
                        break;
                    case ShadingMode::GOURAUD:
                        sceneManager.setLightingModel(LightingModel::GOURAUD);
                        break;
                    case ShadingMode::PHONG:
                        sceneManager.setLightingModel(LightingModel::PHONG);
                        break;
                }
            },
            //controle de projeção
            [this](ProjectionMode mode) {
                switch (mode) {
                    case ProjectionMode::PERSPECTIVE:
                        sceneManager.setProjection(ProjectionType::PERSPECTIVE);
                        break;
                    case ProjectionMode::ORTHOGRAPHIC:
                        sceneManager.setProjection(ProjectionType::ORTHOGRAPHIC);
                        break;
                }
            },
            //controle do tipo do objeto
            [this](ObjectType type) {
                sceneManager.clearObjects(); // Remove objetos extrudados/anteriores
                sceneManager.setObjectType(type);
            },
            //controle de cores
            [this](float r, float g, float b) {
                sceneManager.setObjectColor(r, g, b);
                polygonManager.setFillColor(r, g, b);
                polygonManager.setLineColor(r, g, b);
            },
            
            [this](float r, float g, float b) {
                sceneManager.setLightColor(r, g, b);
            }
        );
        
        //ocorre a definicao dos callbacks para a UI controlar as cenas 2D
        uiManager.set2DCallbacks(
        //edicao dos poligonos como fechar(F)
            [this]() {
                if (polygonManager.getVertexCount() >= 3) {
                    polygonManager.closePolygon();
                    applicationState = ApplicationState::POLYGON_READY;
                }
            },
            //edicao dos poligonos como preencher(P)
            [this]() {
                if (polygonManager.canBeFilled()) {
                    applicationState = ApplicationState::POLYGON_FILLED;
                }
            },
            //edicao dos poligonos como Limpar(C)
            [this]() {
                polygonManager.clearPolygon();
                applicationState = ApplicationState::DRAWING_POLYGON;
            },
            //edicao dos poligonos como salvar(S), alterar vertices(V) e ajustar espessura de linha
            [this]() {
                polygonManager.toggleVertexVisibility();
            },

            [this]() {
                if (polygonManager.canBeFilled()) {
                    bool isFilled = (applicationState == ApplicationState::POLYGON_FILLED);
                    polygonManager.saveCurrentPolygon(isFilled);
                    polygonManager.clearPolygon();
                    applicationState = ApplicationState::DRAWING_POLYGON;
                } else {
                }
            },
           
            [this](bool increase) {
                polygonManager.adjustLineThickness(increase);
            }
        );
        
        eventHandler = new EventHandler(&polygonManager, &graphicsRenderer, &applicationState, 
                                          windowDimensions, &currentMode);
    }

    //criacao dos objetos 3D
    void create3DObjectsFrom2D() {
        sceneManager.clearObjects();
        bool hasObjects = false;

        const auto& savedPolys = polygonManager.getSavedPolygons();
        for (const auto& poly : savedPolys) {
            if (poly.vertices.size() >= 3) {
                //para cada poligono 2D válido, é chamado o sceneManager
                //transformando a forma plana em um objeto 3D com uma determinada profundidade (50.0f)
                sceneManager.createExtrudedObject(poly.vertices, 50.0f);
                hasObjects = true;
            }
        }
        
        if (polygonManager.isPolygonCurrentlyClosed() && polygonManager.getVertexCount() >= 3) {
            sceneManager.createExtrudedObject(polygonManager.getVertices(), 50.0f);
            hasObjects = true;
        }

        //caso nenhum objeto 2D estiver pronto, ele cria um cubo 
        if (!hasObjects) {
            std::vector<Point2D> square;
            square.push_back(Point2D(350, 250));
            square.push_back(Point2D(450, 250));
            square.push_back(Point2D(450, 350));
            square.push_back(Point2D(350, 350));
            sceneManager.createExtrudedObject(square, 100.0f);
        }
    }

    //aqui garante que o objeto seja criado apenas uma vez e que a mesma instância seja sempre retornada, permitindo que
    //qualquer parte do código acesse os dados e os subsistemas centrais de forma global 
    static ApplicationContext* getInstance() {
        static ApplicationContext instance;
        return &instance;
    }
};

#endif // APPLICATION_CONTEXT_H
