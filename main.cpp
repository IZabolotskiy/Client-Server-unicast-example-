#include <QApplication>
#include <QTcpSocket>
#include <QKeyEvent>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLFunctions>

class TriangleWindow : public QWindow, protected QOpenGLFunctions
{
public:
    TriangleWindow()
        : m_program(0)
        , m_vertexBuffer(0)
        , m_vao(0)
        , m_x(0.0f)
        , m_y(0.0f)
        , m_z(0.0f)
    {
        setSurfaceType(QWindow::OpenGLSurface);
        connect(&m_socket, &QTcpSocket::connected, this, &TriangleWindow::onConnected);
        connect(&m_socket, &QTcpSocket::disconnected, this, &TriangleWindow::onDisconnected);
    }

    void initializeGL()
    {
        initializeOpenGLFunctions();

        // Create shader program
        m_program = new QOpenGLShaderProgram(this);
        m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, "#version 330\nin vec3 position;uniform mat4 modelViewProjectionMatrix;void main() {gl_Position = modelViewProjectionMatrix * vec4(position, 1.0);}");
        m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, "#version 330\nout vec4 fragColor;void main() {fragColor = vec4(1.0, 1.0, 1.0, 1.0);}");
        m_program->link();

        // Create vertex buffer
        m_vertexBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        m_vertexBuffer->create();
        m_vertexBuffer->bind();
        GLfloat vertices[] = {
            0.0f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f
        };
        m_vertexBuffer->allocate(vertices, sizeof(vertices));
        m_vertexBuffer->release();

        // Create vertex array object
        m_vao = new QOpenGLVertexArrayObject(this);
        m_vao->create();
        m_vao->bind();
        m_program->bind();
        m_vertexBuffer->bind();
        m_program->enableAttributeArray(0);
        m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3);
        m_vertexBuffer->release();
        m_vao->release();
    }

    void paintGL()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        QMatrix4x4 modelViewMatrix;
        modelViewMatrix.translate(m_x, m_y, m_z);
        modelViewMatrix.rotate(m_rotation);

        QMatrix4x4 projectionMatrix;
        projectionMatrix.perspective(60.0f, width() / (float) height(), 0.1f, 100.0f);

        QMatrix4x4 modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;

        m_program->bind();
        m_vao->bind();
        m_program->setUniformValue("modelViewProjectionMatrix", modelViewProjectionMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        m_vao->release();
        m_program->release();

        glFlush();
    }

    void keyPressEvent(QKeyEvent *event)
    {
        switch (event->key()) {
            case Qt::Key_W:
                m_z -= 0.1f;
                break;
  case Qt::Key_S:
            m_z += 0.1f;
            break;
        case Qt::Key_A:
            m_x -= 0.1f;
            break;
        case Qt::Key_D:
            m_x += 0.1f;
            break;
        case Qt::Key_Q:
            m_y += 0.1f;
            break;
        case Qt::Key_E:
            m_y -= 0.1f;
            break;
        default:
            QWindow::keyPressEvent(event);
    }
}

void onConnected()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << m_x << m_y << m_z;
    m_socket.write(data);
}

void onDisconnected()
{
    // Do nothing
}

void connectToServer()
{
    m_socket.connectToHost("localhost", 8000);
    if (!m_socket.waitForConnected()) {
        qWarning("Failed to connect to server");
    }
}
private:
QOpenGLShaderProgram *m_program;
QOpenGLBuffer *m_vertexBuffer;
QOpenGLVertexArrayObject *m_vao;
QQuaternion m_rotation;
float m_x;
float m_y;
float m_z;
QTcpSocket m_socket;
};

int main(int argc, char *argv[])
{
QApplication app(argc, argv);
TriangleWindow window;
window.resize(800, 600);
window.setTitle("OpenGL Triangle");
window.show();

window.connectToServer();

return app.exec();
}
