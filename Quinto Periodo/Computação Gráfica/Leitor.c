
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
// #include <string.h> // Não é mais necessário para strchr se não for converter vírgulas

#define MAX_VERTICES 10000
#define MAX_NORMALS 10000
#define MAX_FACES 10000

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    int v_idx[3];   // índices dos vértices
    int n_idx[3];   // índices das normais (iguais entre si, pois é uma normal por face)
} Face;

Vec3 vertices[MAX_VERTICES];
Vec3 normals[MAX_NORMALS];
Face faces[MAX_FACES];
int vertex_count = 0, normal_count = 0, face_count = 0;

float scale_factor = 1.0f;
float center_x = 0.0f, center_y = 0.0f, center_z = 0.0f; // Variáveis para o centro do modelo
float max_dim;
int win_id;

// --- Funções auxiliares de vetores ---
Vec3 crossProduct(Vec3 a, Vec3 b) {
    Vec3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

Vec3 normalize(Vec3 v) {
    float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length == 0) return (Vec3){0, 0, 0};
    return (Vec3){v.x / length, v.y / length, v.z / length};
}

// --- Carrega o arquivo OBJ ---
void loadOBJ(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Erro ao abrir arquivo %s\n", filename);
        exit(1);
    }

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            Vec3 v;
            sscanf(line + 2, "%f %f %f", &v.x, &v.y, &v.z);
            vertices[vertex_count++] = v;
        }
        else if (strncmp(line, "f ", 2) == 0) {
            Face f;
            int matches = sscanf(line + 2, "%d %d %d", &f.v_idx[0], &f.v_idx[1], &f.v_idx[2]);
            if (matches == 3) {
                for (int i = 0; i < 3; i++) {
                    f.v_idx[i]--;             // .obj usa indexação 1
                    f.n_idx[i] = face_count; // normal única por face
                }
                faces[face_count++] = f;
            } else {
                printf("Linha de face inválida: %s\n", line);
            }
        }
    }

    fclose(file);
}

// --- Calcula a normal para cada face ---
void computeFaceNormals() {
    for (int i = 0; i < face_count; i++) {
        Vec3 v0 = vertices[faces[i].v_idx[0]];
        Vec3 v1 = vertices[faces[i].v_idx[1]];
        Vec3 v2 = vertices[faces[i].v_idx[2]];

        Vec3 u = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
        Vec3 v_vec = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };

        Vec3 n = normalize(crossProduct(u, v_vec));
        normals[normal_count++] = n;
    }
}

// --- Calcula a escala automática e o centro do modelo ---
void calculateScale() {
    float min_x = 1e9, max_x = -1e9;
    float min_y = 1e9, max_y = -1e9;
    float min_z = 1e9, max_z = -1e9;

    if (vertex_count == 0) { // Garante que há vértices para processar
        scale_factor = 1.0f;
        center_x = center_y = center_z = 0.0f;
        return;
    }

    for (int i = 0; i < vertex_count; i++) {
        Vec3 v = vertices[i];
        if (v.x < min_x) min_x = v.x;
        if (v.x > max_x) max_x = v.x;
        if (v.y < min_y) min_y = v.y;
        if (v.y > max_y) max_y = v.y;
        if (v.z < min_z) min_z = v.z;
        if (v.z > max_z) max_z = v.z;
    }

    float dx = max_x - min_x;
    float dy = max_y - min_y;
    float dz = max_z - min_z;
    max_dim = fmax(fmax(dx, dy), dz);

    scale_factor = (max_dim > 0.0f) ? 40.0f / max_dim : 1.0f;
    printf("max_dim: %f, scale_factor: %f\n", max_dim, scale_factor);

    // Calcula o centro do modelo para centralização
    center_x = (min_x + max_x) / 2.0f;
    center_y = (min_y + max_y) / 2.0f;
    center_z = (min_z + max_z) / 2.0f;

    printf("center: (%f, %f, %f)\n", center_x, center_y, center_z);
}

// --- Renderiza o modelo ---
void drawOBJModel() {
    glPushMatrix();
    glTranslatef(-center_x * scale_factor, -center_y * scale_factor, -center_z * scale_factor);
    glScalef(scale_factor, scale_factor, scale_factor);

    glColor3f(0.0f, 1.0f, 0.0f); // Verde puro
    printf("Vertices: %d, Faces: %d, Normais: %d\n", vertex_count, face_count, normal_count);

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < face_count; i++) {
        for (int j = 0; j < 3; j++) {
            Vec3 v = vertices[faces[i].v_idx[j]];
            glVertex3f(v.x, v.y, v.z);
        }
    }
    glEnd();
    glPopMatrix();
}

// --- Input de teclado normal ---
void myKeyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'R': case 'r': glColor3f(1.0f, 0.0f, 0.0f); break;
        case 'G': case 'g': glColor3f(0.0f, 1.0f, 0.0f); break;
        case 'B': case 'b': glColor3f(0.0f, 0.0f, 1.0f); break;
        case 27: // Tecla ESC
            glutDestroyWindow(win_id);
            exit(0);
    }
    glutPostRedisplay(); // Redesenha a cena
}

// --- Teclas especiais ---
void myKeyboardSpecial(int key, int x, int y) {
    if (key == GLUT_KEY_UP) glutFullScreen();
    else if (key == GLUT_KEY_DOWN) glutReshapeWindow(640, 480);
    glutPostRedisplay(); // Redesenha a cena
}

// --- Mouse ---
void myMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float r = (float)rand() / RAND_MAX;
        float g = (float)rand() / RAND_MAX;
        float b = (float)rand() / RAND_MAX;
        glColor3f(r, g, b);
    }
    glutPostRedisplay(); // Redesenha a cena
}

// --- Inicialização OpenGL ---
void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0); // Fundo preto
    glShadeModel(GL_SMOOTH); // Sombreamento suave
    glEnable(GL_COLOR_MATERIAL); // Habilita a cor do material para usar glColor
    glEnable(GL_NORMALIZE); // Normaliza as normais após transformações de escala

    // Configurações da luz
    GLfloat light_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position[] = { 20.0, 20.0, 20.0, 0.0 }; // Luz direcional

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_LIGHTING); // Habilita o sistema de iluminação
    glEnable(GL_LIGHT0);   // Habilita a luz 0
    glEnable(GL_DEPTH_TEST); // Habilita o teste de profundidade

    glColor3f(0.5f, 1.0f, 0.5f); // Cor inicial do objeto
}

// --- Display ---
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpa o buffer de cor e profundidade
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Posição da câmera para visualização em perspectiva
    // O modelo será transladado para o centro (0,0,0) na função drawOBJModel
    float cam_dist = max_dim * 1.5f; // 1.5 vezes o tamanho maior do ursinho
    gluLookAt(0.0f, 0.0f, cam_dist,
          0.0f, 0.0f, 0.0f,
          0.0f, 1.0f, 0.0f);

    glDisable(GL_LIGHTING);
    drawOBJModel();
    glEnable(GL_LIGHTING);
    

    glutSwapBuffers(); // Troca os buffers para double buffering
}

// --- Reshape ---
void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei) w, (GLsizei) h); // Define a viewport
    glMatrixMode(GL_PROJECTION); // Entra no modo de projeção
    glLoadIdentity(); // Reinicia a matriz de projeção
    // Usa projeção em perspectiva
    gluPerspective(60.0, (GLfloat) w / (GLfloat) h, 1.0, 200.0); // fovy, aspect, near, far
    glMatrixMode(GL_MODELVIEW); // Volta para o modo ModelView
}

// --- Main ---
int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Uso: %s <arquivo.obj>\n", argv[0]);
        return 1;
    }

    loadOBJ(argv[1]); // Carrega o modelo OBJ
    computeFaceNormals(); // Calcula as normais das faces
    calculateScale(); // Calcula a escala e o centro do modelo

    glutInit(&argc, argv);
    // Habilita double buffering para animação mais suave
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowSize(640, 480); // Tamanho inicial da janela
    glutInitWindowPosition(100, 100);
    win_id = glutCreateWindow("Visualizador OBJ");
    init(); // Inicializa OpenGL

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(myKeyboard);
    glutSpecialFunc(myKeyboardSpecial);
    glutMouseFunc(myMouse);
    glutMainLoop(); // Inicia o loop principal do GLUT
    return 0;
}
