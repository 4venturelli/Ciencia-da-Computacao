#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

#define MAX_VERTICES 100000
#define MAX_NORMALS 100000
#define MAX_FACES 100000

typedef struct { float x, y, z; } Vec3;

typedef struct {
    int v[3];
    int vn[3];
} Face;

Vec3 vertices[MAX_VERTICES];
Vec3 normals[MAX_NORMALS];
Face faces[MAX_FACES];
int vertice_num = 0, normal_num = 0, face_num = 0;

Vec3 centro = {0, 0, 0};
float fator_escala = 1.0f;
int win_id;

void processaVertice(char *line) {
    Vec3 v;
    sscanf(line + 2, "%f %f %f", &v.x, &v.y, &v.z);
    vertices[vertice_num++] = v;
}

void processaNormal(char *line) {
    Vec3 n;
    sscanf(line + 3, "%f %f %f", &n.x, &n.y, &n.z);
    normals[normal_num++] = n;
}

void processaFace(char *line) {
    Face f;
    for (int i = 0; i < 3; i++) f.vn[i] = -1; 

    int lidos = sscanf(line + 2, "%d//%d %d//%d %d//%d",
        &f.v[0], &f.vn[0], &f.v[1], &f.vn[1], &f.v[2], &f.vn[2]);

    if (lidos < 6) sscanf(line + 2, "%d %d %d", &f.v[0], &f.v[1], &f.v[2]);

    for (int i = 0; i < 3; i++) {
        f.v[i]--;
        if (f.vn[i] >= 0) f.vn[i]--;
    }

    faces[face_num++] = f;
}

void calculaCentroEscala() {
    float minX=1e9, minY=1e9, minZ=1e9;
    float maxX=-1e9, maxY=-1e9, maxZ=-1e9;

    for (int i = 0; i < vertice_num; i++) {
        Vec3 v = vertices[i];
        if (v.x < minX) minX = v.x; if (v.x > maxX) maxX = v.x;
        if (v.y < minY) minY = v.y; if (v.y > maxY) maxY = v.y;
        if (v.z < minZ) minZ = v.z; if (v.z > maxZ) maxZ = v.z;
    }

    centro.x = (minX + maxX) / 2.0f;
    centro.y = (minY + maxY) / 2.0f;
    centro.z = (minZ + maxZ) / 2.0f;

    float tamanhoX = maxX - minX;
    float tamanhoY = maxY - minY;
    float tamanhoZ = maxZ - minZ;

    float maior = fmaxf(fmaxf(tamanhoX, tamanhoY), tamanhoZ);
    fator_escala = 50.0f / maior;
}

void carregaOBJ(const char *nomeArquivo) {
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo .obj");
        exit(EXIT_FAILURE);
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), arquivo)) {
        for (int i = 0; linha[i]; i++)
            if (linha[i] == ',') linha[i] = '.'; 

        if (strncmp(linha, "v ", 2) == 0)
            processaVertice(linha);
        else if (strncmp(linha, "vn ", 3) == 0)
            processaNormal(linha);
        else if (strncmp(linha, "f ", 2) == 0)
            processaFace(linha);
    }

    fclose(arquivo);
    calculaCentroEscala();
}


Vec3 calculaNormalFace(Vec3 v0, Vec3 v1, Vec3 v2) {
    Vec3 normal;
    float ux = v1.x - v0.x, uy = v1.y - v0.y, uz = v1.z - v0.z;
    float vx = v2.x - v0.x, vy = v2.y - v0.y, vz = v2.z - v0.z;

    normal.x = uy * vz - uz * vy;
    normal.y = uz * vx - ux * vz;
    normal.z = ux * vy - uy * vx;

    float len = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (len > 0.0001f) {
        normal.x /= len;
        normal.y /= len;
        normal.z /= len;
    }
    return normal;
}

void desenhaOBJ(){
    glPushMatrix();
    glScalef(fator_escala, fator_escala, fator_escala);
    glTranslatef(-centro.x, -centro.y, -centro.z);

    for (int i = 0; i < face_num; i++) {
        Face face = faces[i];

        Vec3 v0 = vertices[face.v[0]];
        Vec3 v1 = vertices[face.v[1]];
        Vec3 v2 = vertices[face.v[2]];

        Vec3 normal;
        int usa_normais_arquivo = (face.vn[0] >= 0 && face.vn[1] >= 0 && face.vn[2] >= 0);

        if (!usa_normais_arquivo) {
            normal = calculaNormalFace(v0, v1, v2);
        }

        glBegin(GL_TRIANGLES);
        for (int j = 0; j < 3; j++) {
            Vec3 v = vertices[face.v[j]];
            if (usa_normais_arquivo) {
                Vec3 n = normals[face.vn[j]];
                glNormal3f(n.x, n.y, n.z);
            } else {
                glNormal3f(normal.x, normal.y, normal.z);
            }
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
    }

    glPopMatrix();
}

void myKeyboard(unsigned char key, int x, int y)
{
  switch (key) {
    case 'R': 
    case 'r':// muda a cor corrente para vermelho
      glColor3f(1.0f, 0.0f, 0.0f);
      break;
    case 'G':
    case 'g':// muda a cor corrente para verde
      glColor3f(0.0f, 1.0f, 0.0f);
      break;
    case 'B':
    case 'b':// muda a cor corrente para azul
      glColor3f(0.0f, 0.0f, 1.0f);
      break;
    //case 27:
    //  glutDestroyWindow(win_id);
    //  exit(0);
    //  break;
  }
  glutPostRedisplay();
}

//Aqui se capturam as teclas especiais (Alt, Control, Shift, F1, F2, etc.)
void myKeyboardSpecial(int key, int x, int y ) {
  switch ( key ) {
    case GLUT_KEY_UP:                   // Quando a seta para cima é teclada...
      glutFullScreen ( );               // Vá para o modo tela cheia...
      break;
    case GLUT_KEY_DOWN:                 // Quando a seta para baixo for teclada...
      glutReshapeWindow ( 640, 480 );   // Vá para modo em janela de 640 por 480
      break;
    default:
      printf("Você apertou a tecla especial código: %d\n", key);  // ...para ajudar você a debugar...       
      break;
  }
}

// Função callback chamada para gerenciar eventos do mouse
void myMouse(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON)
    if (state == GLUT_DOWN) {
      float r, g, b;
      r = (double)rand() / (double)RAND_MAX;
      g = (double)rand() / (double)RAND_MAX;
      b = (double)rand() / (double)RAND_MAX;
      glColor3f(r,g,b);
      printf("%.2f, %.2f, %.2f, na posicao %d, %d\n", r, g, b, x, y);
    }
  glutPostRedisplay();
}

void init(void) {
    // Define a cor de fundo da janela de visualização como preto
    glClearColor (0.0, 0.0, 0.0, 0.0);
    // Define o modo de rastreio de cor utilizado para misturar a cor do material nos pontos da face (smoothing)
    glShadeModel (GL_SMOOTH);
    // Habilita a definição da cor do material a partir da cor corrente
    glEnable(GL_COLOR_MATERIAL);
    // Habilita a normalização do vetor normal na função glNormal
    glEnable(GL_NORMALIZE);

    //Parâmetros para a luz ambiente, difusa e especular, definindo também a posição da luz
    GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position[] = { 10.0, 10.0, 10.0, 0.0 };

    // Passa os parâmetros definidos acima para a OpenGL
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    // Habilita a luz ambiente, a luz zero e o depth buffer
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    //Primeira cor, para depois mudarmos com os eventos
    glColor3f(0.5, 1.0, 0.5);

}


void display(void) {
    // Limpa a janela de visualização com a cor de fundo especificada, e limpa também o depth buffer
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Entra no modo de definição de matriz de visualização
    glMatrixMode(GL_MODELVIEW);
    // Carrega a matriz identidade
    glLoadIdentity();
    // Define a posição da câmera, para onde ela aponta e qual o vetor UP
    gluLookAt(0.0f, 30.0f, 50.0f, 0.0f, 20.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    
    desenhaOBJ();
    //glutSolidTeapot(10.0);
    
    // Executa os comandos OpenGL, e depois troca os buffers de vídeo
    glFlush();
}

void reshape(int w, int h) {
    // Define o viewport como o tamanho da janela
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
    // Entra no modo de definição de matriz de projeção
    glMatrixMode (GL_PROJECTION);
    // Carrega a matriz identidade
    glLoadIdentity();
    // Define a projeção ortogonal
    if (w <= h)
    // Se a largura for menor que a altura, a projeção é feita para manter a proporção da janela
    glOrtho (-50, 50, -50*(GLfloat)h/(GLfloat)w, 50*(GLfloat)h/(GLfloat)w, -100.0, 100.0);
    else
    // Se a altura for menor que a largura, a projeção é feita para manter a proporção da janela
    glOrtho (-50*(GLfloat)w/(GLfloat)h, 50*(GLfloat)w/(GLfloat)h, -50, 50, -100.0, 100.0);
}

int main(int argc, char **argv) {
    if (argc < 2) { printf("Uso: %s arquivo.obj\n", argv[0]); return 1; }
    carregaOBJ(argv[1]);
    // Inicializa a biblioteca GLUT e negocia uma seção com o gerenciador de janelas
    glutInit(&argc, argv);
    // Inicializa o display, indicando que usará:
    // - um buffer de imagem (single buffer);
    // - buffer de profundidade;
    // - padrão de cores RGB)
    glutInitDisplayMode (GLUT_SINGLE | GLUT_DEPTH | GLUT_RGB);
    // Especifica as dimensões da janela de exibição
    glutInitWindowSize (500, 500); 
    // Especifica a posição inicial da janela de exibição
    glutInitWindowPosition (100, 100);
    // Cria a janela de exibição
    win_id = glutCreateWindow("Visualizador OBJ");
    // Chama a função init
    init ();
    // Estabelece a função "display" como a função callback de exibição.
    glutDisplayFunc(display); 
    // Estabelece a função "reshape" como a função callback de redimensionamento da janela de exibição.
    glutReshapeFunc(reshape);
    // Estabelece a função "keyboard" como a função callback de evento de teclado.
    glutKeyboardFunc(myKeyboard);
    // Estabelece a função "keyboardSpecial" como a função callback de evento de teclado especial.
    glutSpecialFunc(myKeyboardSpecial);
    // Estabelece a função "mouse" como a função callback de evento de mouse.
    glutMouseFunc(myMouse);
    // Entra no loop de eventos, não retorna
    glutMainLoop();
  return 0;
}