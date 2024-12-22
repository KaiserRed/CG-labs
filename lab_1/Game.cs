using OpenTK.Graphics.OpenGL4;
using OpenTK.Windowing.Common;
using OpenTK.Windowing.Desktop;
using OpenTK.Mathematics;
using System;

namespace BezierCurve
{
    class Game : GameWindow
    {
        private int _shaderProgram;
        private int _vao, _vbo;
        private Vector2[] _controlPoints = new Vector2[3];
        private int _selectedPoint = -1; // -1 means no point selected

        public Game(int width, int height, string title) 
            : base(GameWindowSettings.Default, new NativeWindowSettings { ClientSize = new Vector2i(width, height), Title = title }) { }

        protected override void OnLoad()
        {
            base.OnLoad();

            GL.ClearColor(0.2f, 0.3f, 0.3f, 1.0f);

            // Initialize control points
            _controlPoints[0] = new Vector2(-0.5f, -0.5f);
            _controlPoints[1] = new Vector2(0.0f, 0.5f);
            _controlPoints[2] = new Vector2(0.5f, -0.5f);

            // Initialize shaders
            _shaderProgram = CreateShaderProgram();

            // Initialize VAO and VBO
            _vao = GL.GenVertexArray();
            _vbo = GL.GenBuffer();

            GL.BindVertexArray(_vao);
            GL.BindBuffer(BufferTarget.ArrayBuffer, _vbo);
            GL.BufferData(BufferTarget.ArrayBuffer, sizeof(float) * 2 * 3, IntPtr.Zero, BufferUsageHint.DynamicDraw);

            GL.VertexAttribPointer(0, 2, VertexAttribPointerType.Float, false, 2 * sizeof(float), 0);
            GL.EnableVertexAttribArray(0);
        }

        protected override void OnRenderFrame(FrameEventArgs args)
        {
            base.OnRenderFrame(args);

            GL.Clear(ClearBufferMask.ColorBufferBit);

            // Update VBO with control points
            GL.BindBuffer(BufferTarget.ArrayBuffer, _vbo);
            GL.BufferSubData(BufferTarget.ArrayBuffer, IntPtr.Zero, sizeof(float) * 2 * 3, ref _controlPoints[0]);

            // Draw control points
            GL.UseProgram(_shaderProgram);
            GL.PointSize(10.0f);
            GL.DrawArrays(PrimitiveType.Points, 0, 3);

            // Draw Bézier curve
            DrawBezierCurve();

            SwapBuffers();
        }

        private void DrawBezierCurve()
        {
            const int segments = 100;
            Vector2[] bezierPoints = new Vector2[segments + 1];

            for (int i = 0; i <= segments; i++)
            {
                float t = i / (float)segments;
                bezierPoints[i] = (1 - t) * (1 - t) * _controlPoints[0] +
                                  2 * (1 - t) * t * _controlPoints[1] +
                                  t * t * _controlPoints[2];
            }

            GL.BufferData(BufferTarget.ArrayBuffer, sizeof(float) * 2 * (segments + 1), bezierPoints, BufferUsageHint.DynamicDraw);
            GL.DrawArrays(PrimitiveType.LineStrip, 0, segments + 1);
        }

        protected override void OnMouseDown(MouseButtonEventArgs e)
        {
            base.OnMouseDown(e);

            var mousePos = ConvertScreenToWorld(MousePosition);
            for (int i = 0; i < _controlPoints.Length; i++)
            {
                if (Vector2.Distance(mousePos, _controlPoints[i]) < 0.05f)
                {
                    _selectedPoint = i;
                    break;
                }
            }
        }

        protected override void OnMouseUp(MouseButtonEventArgs e)
        {
            base.OnMouseUp(e);
            _selectedPoint = -1; // Deselect point
        }

        protected override void OnMouseMove(MouseMoveEventArgs e)
        {
            base.OnMouseMove(e);

            if (_selectedPoint != -1)
            {
                var mousePos = ConvertScreenToWorld(MousePosition);
                _controlPoints[_selectedPoint] = mousePos;
            }
        }

        private Vector2 ConvertScreenToWorld(Vector2 screenPos)
        {
            float x = 2.0f * screenPos.X / ClientSize.X - 1.0f;
            float y = 1.0f - 2.0f * screenPos.Y / ClientSize.Y;
            return new Vector2(x, y);
        }

        private int CreateShaderProgram()
        {
            string vertexShaderSource = @"
                #version 330 core
                layout(location = 0) in vec2 aPos;
                void main()
                {
                    gl_Position = vec4(aPos, 0.0, 1.0);
                }
            ";

            string fragmentShaderSource = @"
                #version 330 core
                out vec4 FragColor;
                void main()
                {
                    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
                }
            ";

            int vertexShader = GL.CreateShader(ShaderType.VertexShader);
            GL.ShaderSource(vertexShader, vertexShaderSource);
            GL.CompileShader(vertexShader);
            CheckCompileErrors(vertexShader, "VERTEX");

            int fragmentShader = GL.CreateShader(ShaderType.FragmentShader);
            GL.ShaderSource(fragmentShader, fragmentShaderSource);
            GL.CompileShader(fragmentShader);
            CheckCompileErrors(fragmentShader, "FRAGMENT");

            int shaderProgram = GL.CreateProgram();
            GL.AttachShader(shaderProgram, vertexShader);
            GL.AttachShader(shaderProgram, fragmentShader);
            GL.LinkProgram(shaderProgram);
            CheckLinkErrors(shaderProgram);

            GL.DeleteShader(vertexShader);
            GL.DeleteShader(fragmentShader);

            return shaderProgram;
        }

        private void CheckCompileErrors(int shader, string type)
        {
            GL.GetShader(shader, ShaderParameter.CompileStatus, out int success);
            if (success == 0)
            {
                string infoLog = GL.GetShaderInfoLog(shader);
                throw new Exception($"ERROR::SHADER_COMPILATION_ERROR of type: {type}\n{infoLog}");
            }
        }

        private void CheckLinkErrors(int program)
        {
            GL.GetProgram(program, GetProgramParameterName.LinkStatus, out int success);
            if (success == 0)
            {
                string infoLog = GL.GetProgramInfoLog(program);
                throw new Exception($"ERROR::PROGRAM_LINKING_ERROR\n{infoLog}");
            }
        }

        protected override void OnUnload()
        {
            base.OnUnload();
            GL.DeleteBuffer(_vbo);
            GL.DeleteVertexArray(_vao);
            GL.DeleteProgram(_shaderProgram);
        }
    }
}
