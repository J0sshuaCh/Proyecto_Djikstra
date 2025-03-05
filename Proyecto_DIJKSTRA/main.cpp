#include "include/glad.h"
#include <GLFW/glfw3.h>
#include <GL/glut.h>  // GLUT para renderizar texto
#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <limits>
#include <thread>
#include <chrono>
#include <string>

using namespace std;

// Estructura para representar un nodo con ID
struct Nodo {
    float x, y;
    int id;
    bool visitado = false;      // Para Dijkstra: se pinta en verde si fue visitado
    bool seleccionado = false;  // Se usa para indicar la selección en modo Dijkstra
};

// Variables globales
vector<Nodo> nodos;
vector<pair<int, int>> aristasDirigidas;
vector<float> distanciasAristas;      // Guarda la distancia de cada arista
vector<pair<int, int>> caminoMasCorto;  // Aristas que conforman el camino más corto

// Declaraciones anticipadas (forward declarations)
float calcularDistancia(float x1, float y1, float x2, float y2);
void dijkstraVisual(int inicio, int destino);
void dibujarCirculo(float x, float y, float radio, bool visitado, bool seleccionado);
void dibujarFlecha(float x1, float y1, float x2, float y2, bool enCaminoMasCorto);
void dibujarTexto(const string &texto, float x, float y);
void dibujarGrafo();
pair<float, float> obtenerCoordenadasOpenGL(double xpos, double ypos, int width, int height);
void mouseCallback(GLFWwindow* window, int button, int action, int mods);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void inicializarOpenGL();


// Variables para el modo Dijkstra
// modoSeleccion = 0: modo normal; 1: esperando seleccionar nodo de inicio; 2: esperando seleccionar nodo de destino
int modoSeleccion = 0;
int nodoInicioSeleccionado = -1;
int nodoDestinoSeleccionado = -1;
bool ejecutandoDijkstra = false;

// Variable para seleccionar aristas en modo normal
int primerNodoSeleccionado = -1;  // Para crear aristas entre dos nodos

// Función para calcular la distancia Euclidiana entre dos puntos (multiplicada por 100 y redondeada)
float calcularDistancia(float x1, float y1, float x2, float y2) {
    return round(sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2)) * 100);
}

// Función para convertir un índice en una letra (A, B, C, D, etc.)
char obtenerLetraDeNodo(int indice) {
    return 'A' + indice;  // Convierte el índice en una letra (0 -> A, 1 -> B, etc.)
}

// Función para dibujar una flecha candidata (en amarillo) que muestre la arista en prueba
void dibujarFlechaCandidate(float x1, float y1, float x2, float y2) {
    glColor3f(1.0f, 1.0f, 1.0f);  // Amarillo
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}


// Función para ejecutar Dijkstra paso a paso (visual)
void dijkstraVisual(int inicio, int destino) {
    int n = nodos.size();
    vector<float> dist(n, numeric_limits<float>::infinity());
    vector<int> prev(n, -1);
    // Cola de prioridad (min-heap)
    priority_queue<pair<float, int>, vector<pair<float, int>>, greater<>> pq;

    dist[inicio] = 0;
    pq.push({0, inicio});
    ejecutandoDijkstra = true;

    // Mientras haya nodos por procesar
    while (!pq.empty()) {
        int u = pq.top().second;
        pq.pop();
          // Si llegamos al destino, detenemos

        nodos[u].visitado = true;  // Marca el nodo como visitado

        // Procesa todas las aristas salientes de 'u'
        for (size_t i = 0; i < aristasDirigidas.size(); i++) {
            if (aristasDirigidas[i].first == u) {
                int v = aristasDirigidas[i].second;
                float peso = distanciasAristas[i];
                // Dentro de dijkstraVisual, en el bucle que recorre las aristas salientes de 'u'
            if (dist[u] + peso < dist[v]) {
                // Mostrar visualmente la arista candidata en amarillo
                dibujarFlechaCandidate(nodos[u].x, nodos[u].y, nodos[v].x, nodos[v].y);
                glfwPollEvents();
                this_thread::sleep_for(chrono::milliseconds(300));

                // Actualizar la distancia y el nodo previo
                dist[v] = dist[u] + peso;
                prev[v] = u;
                pq.push({dist[v], v});

                // Marcar temporalmente el nodo 'v' (por ejemplo, en modo "seleccionado")
                nodos[v].seleccionado = true;
                dibujarGrafo();
                glfwPollEvents();
                this_thread::sleep_for(chrono::milliseconds(300));
                nodos[v].seleccionado = false;
            }
            }
        }
        // Actualiza la visualización
        glfwPollEvents();
        this_thread::sleep_for(chrono::milliseconds(500));
    }

    // Reconstruir el camino más corto desde el destino hacia el inicio
    caminoMasCorto.clear();
    int actual = destino;
    while (actual != -1 && prev[actual] != -1) {
        caminoMasCorto.push_back({prev[actual], actual});
        actual = prev[actual];
    }
    ejecutandoDijkstra = false;
}

// Dibuja un círculo (nodo)
// Si el nodo está seleccionado (modo Dijkstra) se pinta en naranja
void dibujarCirculo(float x, float y, float radio, bool visitado, bool seleccionado) {
    if (seleccionado)
        glColor3f(1.0f, 0.5f, 0.0f);  // Naranja
    else
        glColor3f(visitado ? 0.2f : 0.2f, visitado ? 1.0f : 0.8f, 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 100; i++) {
        float angulo = 2.0f * M_PI * i / 100;
        glVertex2f(x + cos(angulo) * radio, y + sin(angulo) * radio);
    }
    glEnd();
}

// Dibuja una línea (arista). Si la arista forma parte del camino más corto se pinta en rojo; si no, en blanco.
void dibujarFlecha(float x1, float y1, float x2, float y2, bool enCaminoMasCorto) {
    glColor3f(enCaminoMasCorto ? 1.0f : 1.0f, enCaminoMasCorto ? 0.0f : 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

// Dibuja texto (letra del nodo) usando GLUT
void dibujarTexto(const string &texto, float x, float y) {
    glColor3f(1.0f, 1.0f, 1.0f);  // Color blanco para el texto
    glRasterPos2f(x, y);
    for (char c : texto) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

// Renderiza el grafo
void dibujarGrafo() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Dibuja aristas y, sobre ellas, la distancia
    for (size_t i = 0; i < aristasDirigidas.size(); i++) {
        int u = aristasDirigidas[i].first;
        int v = aristasDirigidas[i].second;
        bool enCamino = false;
        for (const auto &arista : caminoMasCorto) {
            if (arista.first == u && arista.second == v) {
                enCamino = true;
                break;
            }
        }
        dibujarFlecha(nodos[u].x, nodos[u].y, nodos[v].x, nodos[v].y, enCamino);
        float midX = (nodos[u].x + nodos[v].x) / 2;
        float midY = (nodos[u].y + nodos[v].y) / 2;
        dibujarTexto(to_string((int)distanciasAristas[i]), midX, midY);
    }

    // Dibuja nodos y sus letras en lugar de los IDs
for (const auto &nodo : nodos) {
    bool resaltar = (nodo.id == nodoInicioSeleccionado || nodo.id == nodoDestinoSeleccionado);

    // Llama a obtenerLetraDeNodo para convertir el índice del nodo a letra
    char letra = obtenerLetraDeNodo(nodo.id);

    // Dibuja el nodo como una letra en lugar de un ID numérico
    dibujarCirculo(nodo.x, nodo.y, 0.05f, nodo.visitado, resaltar);
    dibujarTexto(string(1, letra), nodo.x - 0.02f, nodo.y + 0.06f);  // Pasamos la letra
}

    glfwSwapBuffers(glfwGetCurrentContext());
}

// Convierte coordenadas de pantalla a coordenadas OpenGL
pair<float, float> obtenerCoordenadasOpenGL(double xpos, double ypos, int width, int height) {
    float x = (xpos / width) * 2.0f - 1.0f;
    float y = 1.0f - (ypos / height) * 2.0f;
    return {x, y};
}

// Manejo de clics del mouse
// Si estamos en modo Dijkstra (modoSeleccion != 0), los clics se usan para seleccionar nodos de inicio y destino.
// En modo normal, clic izquierdo crea nodo y clic derecho conecta nodos.
void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action != GLFW_PRESS) return;
    double xpos, ypos;
    int width, height;
    glfwGetCursorPos(window, &xpos, &ypos);
    glfwGetWindowSize(window, &width, &height);
    auto [x, y] = obtenerCoordenadasOpenGL(xpos, ypos, width, height);

    // Si estamos en modo Dijkstra: los clics seleccionan nodos para inicio y destino.
    if (modoSeleccion != 0) {
        // Buscar el nodo más cercano al clic
        int nodoCercano = -1;
        float distanciaMin = 0.1f;
        for (int i = 0; i < nodos.size(); i++) {
            float dx = nodos[i].x - x;
            float dy = nodos[i].y - y;
            float distancia = sqrt(dx * dx + dy * dy);
            if (distancia < distanciaMin) {
                distanciaMin = distancia;
                nodoCercano = i;
            }
        }
        if (nodoCercano != -1) {
            if (modoSeleccion == 1) { // Seleccionar nodo de inicio
                nodoInicioSeleccionado = nodoCercano;
                cout << "Nodo de inicio seleccionado: " << nodoInicioSeleccionado << endl;
                modoSeleccion = 2;  // Ahora se espera seleccionar destino
            } else if (modoSeleccion == 2) { // Seleccionar nodo de destino
                nodoDestinoSeleccionado = nodoCercano;
                cout << "Nodo de destino seleccionado: " << nodoDestinoSeleccionado << endl;
                modoSeleccion = 0;  // Salir del modo selección
                // Ejecuta Dijkstra en un hilo separado para no bloquear el renderizado
                dijkstraVisual(nodoInicioSeleccionado, nodoDestinoSeleccionado);
            }
        }
        return;
    }

    // Modo normal: 
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        nodos.push_back({x, y, (int)nodos.size(), false});
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        static int primerSeleccionado = -1;
        int nodoCercano = -1;
        float distanciaMin = 0.1f;
        for (int i = 0; i < nodos.size(); i++) {
            float dx = nodos[i].x - x;
            float dy = nodos[i].y - y;
            float distancia = sqrt(dx * dx + dy * dy);
            if (distancia < distanciaMin) {
                distanciaMin = distancia;
                nodoCercano = i;
            }
        }
        if (nodoCercano != -1) {
            if (primerSeleccionado == -1) {
                primerSeleccionado = nodoCercano;
            } else {
                if (primerSeleccionado != nodoCercano) {
                    aristasDirigidas.push_back({primerSeleccionado, nodoCercano});
                    distanciasAristas.push_back(calcularDistancia(nodos[primerSeleccionado].x, nodos[primerSeleccionado].y,
                                                                  nodos[nodoCercano].x, nodos[nodoCercano].y));
                }
                primerSeleccionado = -1;
            }
        }
    }
}

// Manejo de teclado: al presionar "D" se activa el modo Dijkstra para seleccionar inicio y destino
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_D && !ejecutandoDijkstra) {
            modoSeleccion = 1;
            cout << "Modo Dijkstra activado. Haga clic en el nodo de INICIO y luego en el de DESTINO." << endl;
        }
        else if (key == GLFW_KEY_C) {
            // Limpiar el grafo
            nodos.clear();
            aristasDirigidas.clear();
            distanciasAristas.clear();
            caminoMasCorto.clear();
            primerNodoSeleccionado = -1;
            nodoInicioSeleccionado = -1;
            nodoDestinoSeleccionado = -1;
            modoSeleccion = 0;
            cout << "Grafo limpio. Puedes crear uno nuevo." << endl;
        }
    }
}

// Inicializa OpenGL
void inicializarOpenGL() {
    glClearColor(0.1, 0.1, 0.1, 1);
}

// Función principal
int main(int argc, char** argv) {
    glutInit(&argc, argv);  // Inicializa GLUT para el texto
    if (!glfwInit()) {
        cerr << "Error al inicializar GLFW" << endl;
        return -1;
    }
    GLFWwindow* ventana = glfwCreateWindow(800, 800, "Dijkstra Visual Interactivo", NULL, NULL);
    if (!ventana) {
        cerr << "Error al crear la ventana GLFW" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(ventana);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    inicializarOpenGL();
    glfwSetMouseButtonCallback(ventana, mouseCallback);
    glfwSetKeyCallback(ventana, keyCallback);

    while (!glfwWindowShouldClose(ventana)) {
        dibujarGrafo();
        glfwPollEvents();
    }

    glfwDestroyWindow(ventana);
    glfwTerminate();
    return 0;
}
