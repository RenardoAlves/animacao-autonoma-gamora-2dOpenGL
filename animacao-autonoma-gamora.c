#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>

void CALLBACK beginCallback(GLenum type) {
    glBegin(type);
}

void CALLBACK endCallback() {
    glEnd();
}

void CALLBACK vertexCallback(void *vertexData) {
    const GLdouble *ptr = (GLdouble *) vertexData;
    glVertex2dv(ptr);
}

void CALLBACK errorCallback(GLenum errorCode) {
    const GLubyte *err = gluErrorString(errorCode);
    printf("Tessellation Error: %s\n", err);
}

void CALLBACK combineCallback(GLdouble coords[3], void *data[4],
                              GLfloat weight[4], void **outData)
{
    GLdouble *vertex = (GLdouble *)malloc(3 * sizeof(GLdouble));
    vertex[0] = coords[0];
    vertex[1] = coords[1];
    vertex[2] = coords[2];
    *outData = vertex;
}

// --- Estruturas ---
typedef struct {
    float x, y;
} Ponto;

typedef struct {
    int n;                 // número de vértices
    unsigned char r, g, b; // cor (0-255)
    Ponto *v;              // vetor de vértices
} Poligono;

typedef struct {
    Poligono bracoEsq;
    Poligono bracoDir;
    Poligono pernaEsq;
    Poligono pernaDir;
    Poligono corpo;
    Poligono rosto;
    Poligono cabelo;
} Personagem;

typedef struct {
    Poligono chao;
    Poligono montanhas;
    Poligono estrela;
    Poligono ceu;
    Poligono sol;
    Poligono iglu;
    Poligono portaIglu;
} Cenario;

// --- Ângulos para os movimentos gerais ---
float tx = 10.0f, ty = -30.0f;
float anguloGeral = 0.0f;
float escala = 1.0f;
float passoTrans = 0.2f;
float passoRot = 5.0f;
float passoEscala = 0.05f;

// --- Ângulos locais das articulações ---
float angBracoEsq = 30, angBracoDir = -80;
float angPernaEsq = 10, angPernaDir = -10;
float angCabeca = 0;

// --- Funções ---
Poligono LerPoligono(const char *nomeArquivo) {
    FILE *f = fopen(nomeArquivo, "r");
    if (!f) {
        printf("Erro ao abrir %s!\n", nomeArquivo);
        exit(1);
    }

    Poligono p;
    fscanf(f, "%d %hhu %hhu %hhu", &p.n, &p.r, &p.g, &p.b);
    p.v = malloc(p.n * sizeof(Ponto));
    for (int i = 0; i < p.n; i++)
        fscanf(f, "%f %f", &p.v[i].x, &p.v[i].y);

    fclose(f);
    return p;
}

void DesenhaPoligono(Poligono p) {
    GLUtesselator *tess = gluNewTess();

    // callbacks
    gluTessCallback(tess, GLU_TESS_BEGIN, (void (CALLBACK*)()) beginCallback);
    gluTessCallback(tess, GLU_TESS_END,   (void (CALLBACK*)()) endCallback);
    gluTessCallback(tess, GLU_TESS_VERTEX,(void (CALLBACK*)()) vertexCallback);
    gluTessCallback(tess, GLU_TESS_ERROR, (void (CALLBACK*)()) errorCallback);
    gluTessCallback(tess, GLU_TESS_COMBINE, (void (CALLBACK*)()) combineCallback);


    glColor3ub(p.r, p.g, p.b);

    gluTessBeginPolygon(tess, NULL);
    gluTessBeginContour(tess);

    // precisamos passar double e manter os ponteiros
    double (*coords)[3] = malloc(p.n * sizeof(double[3]));

    for (int i = 0; i < p.n; i++) {
        coords[i][0] = p.v[i].x;
        coords[i][1] = p.v[i].y;
        coords[i][2] = 0.0;   // z sempre 0
        gluTessVertex(tess, coords[i], coords[i]);
    }

    gluTessEndContour(tess);
    gluTessEndPolygon(tess);
    gluDeleteTess(tess);
    free(coords);

    // --- contorno ---
    glColor3ub(0, 0, 0);
    glLineWidth(1);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < p.n; i++)
        glVertex2f(p.v[i].x, p.v[i].y);
    glEnd();
}

void DesenhaCenario(Cenario c){
    DesenhaPoligono(c.ceu);
    glPushMatrix();
        glTranslatef(-40.0, 0.0, 0.0);
        DesenhaPoligono(c.montanhas);
        DesenhaPoligono(c.chao);
        DesenhaPoligono(c.sol);
        DesenhaPoligono(c.estrela);
        DesenhaPoligono(c.iglu);
        DesenhaPoligono(c.portaIglu);
    glPopMatrix();
}

// --- Desenha personagem hierarquicamente ---
void DesenhaPersonagemHier(Personagem p) {
    glPushMatrix();

        glTranslatef(tx, ty, 0);
        glRotatef(anguloGeral, 0, 0, 1);
        glScalef(escala, escala, 1);

        // Braço direito
        glPushMatrix();
            glTranslatef(1.69, 8.23, 0); // ombro
            glRotatef(angBracoDir, 0, 0, 1); // Ângulo para rotação no ombro dir.
            DesenhaPoligono(p.bracoDir);
        glPopMatrix();

        // Perna direita
        glPushMatrix();
            glTranslatef(0.86, 1.98, 0); // quadril
            glRotatef(angPernaDir, 0, 0, 1); // Ângulo para rotação da perna dir.
            DesenhaPoligono(p.pernaDir);
        glPopMatrix();

        // Perna esquerda
        glPushMatrix();
            glTranslatef(0.86, 1.98, 0); // quadril
            glRotatef(angPernaEsq, 0, 0, 1); // Ângulo para rotação da perna esq.
            DesenhaPoligono(p.pernaEsq);
        glPopMatrix();

        glTranslatef(0.54, 5.40, 0);
        DesenhaPoligono(p.corpo);

        // Braço esquerdo
        glPushMatrix();
            glTranslatef(1.15, 2.83, 0); // ombro
            glRotatef(angBracoEsq, 0, 0, 1); // Ângulo para rotação no ombro esq.
            DesenhaPoligono(p.bracoEsq);
        glPopMatrix();

        // Cabeça (posição relativa ao corpo)
        glPushMatrix();
            glTranslatef(0.81, 4.33, 0);
            glRotatef(angCabeca, 0, 0, 1); // Ângulo para rotação no pescoço
            DesenhaPoligono(p.rosto);
            DesenhaPoligono(p.cabelo);
        glPopMatrix();

    glPopMatrix();
}

// --- Variável global ---
Personagem personagem;
Cenario cenario;

// --- Inicialização OpenGL ---
void Inicializa(void) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-50, 50, -50, 50);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// --- Display ---
void Display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    DesenhaCenario(cenario);
    DesenhaPersonagemHier(personagem);
    glutSwapBuffers();
}

// Função de callback para redimensionamento
void Redimensionar(int w, int h) {
    if (w == h) {
        glViewport(0, 0, w, h);
    } else if (w > h) {
        int x = (w - h) / 2;
        glViewport(x, 0, h, h);
    } else {
        int y = (h - w) / 2;
        glViewport(0, y, w, w);
    }
}

// --- Programa principal ---
int main(int argc, char **argv) {

    personagem.bracoEsq = LerPoligono("braco.txt");
    personagem.bracoDir = LerPoligono("braco.txt");
    personagem.pernaEsq = LerPoligono("perna.txt");
    personagem.pernaDir = LerPoligono("perna.txt");
    personagem.corpo = LerPoligono("corpo.txt");
    personagem.rosto = LerPoligono("rosto.txt");
    personagem.cabelo = LerPoligono("cabelo.txt");
    cenario.estrela = LerPoligono("estrela.txt");
    cenario.ceu = LerPoligono("ceu.txt");
    cenario.sol = LerPoligono("sol.txt");
    cenario.iglu = LerPoligono("iglu.txt");
    cenario.portaIglu = LerPoligono("iglu_porta.txt");
    cenario.montanhas = LerPoligono("montanhas.txt");
    cenario.chao = LerPoligono("chao.txt");

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1000, 1000);
    glutCreateWindow("Personagem Hierarquico");
    Inicializa();
    glutDisplayFunc(Display);
    glutReshapeFunc(Redimensionar);
    glutMainLoop();

    return 0;
}