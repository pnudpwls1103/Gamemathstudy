
#include "Precompiled.h"
#include "SoftRenderer.h"
#include <random>
using namespace CK::DD;

// ���ڸ� �׸��� �Լ�
void SoftRenderer::DrawGizmo2D()
{
	auto& r = GetRenderer();
	const auto& g = Get2DGameEngine();

	// �׸��� ����
	LinearColor gridColor(LinearColor(0.8f, 0.8f, 0.8f, 0.3f));

	// ���� ���� ���
	Vector2 viewPos = g.GetMainCamera().GetTransform().GetPosition();
	Vector2 extent = Vector2(_ScreenSize.X * 0.5f, _ScreenSize.Y * 0.5f);

	// ���� �ϴܿ������� ���� �׸���
	int xGridCount = _ScreenSize.X / _Grid2DUnit;
	int yGridCount = _ScreenSize.Y / _Grid2DUnit;

	// �׸��尡 ���۵Ǵ� ���ϴ� ��ǥ �� ���
	Vector2 minPos = viewPos - extent;
	Vector2 minGridPos = Vector2(ceilf(minPos.X / (float)_Grid2DUnit), ceilf(minPos.Y / (float)_Grid2DUnit)) * (float)_Grid2DUnit;
	ScreenPoint gridBottomLeft = ScreenPoint::ToScreenCoordinate(_ScreenSize, minGridPos - viewPos);

	for (int ix = 0; ix < xGridCount; ++ix)
	{
		r.DrawFullVerticalLine(gridBottomLeft.X + ix * _Grid2DUnit, gridColor);
	}

	for (int iy = 0; iy < yGridCount; ++iy)
	{
		r.DrawFullHorizontalLine(gridBottomLeft.Y - iy * _Grid2DUnit, gridColor);
	}

	ScreenPoint worldOrigin = ScreenPoint::ToScreenCoordinate(_ScreenSize, -viewPos);
	r.DrawFullHorizontalLine(worldOrigin.Y, LinearColor::Red);
	r.DrawFullVerticalLine(worldOrigin.X, LinearColor::Green);
}

// ���� ������Ʈ ���


// ���� �� �ε��� ����ϴ� �Լ�
void SoftRenderer::LoadScene2D()
{
	// ���� �� �ε����� ����ϴ� ��� �� �ֿ� ���۷���
	auto& g = Get2DGameEngine();

}

// ���� ������ ������ ������ �����ϴ� ����
float currentShear = 0.f;
float currentScale = 10.f;
float currentDegree = 0.f;

// ���� ������ ����ϴ� �Լ�
void SoftRenderer::Update2D(float InDeltaSeconds)
{
	// ���� �������� ����ϴ� ��� �� �ֿ� ���۷���
	auto& g = Get2DGameEngine();
	const InputManager& input = g.GetInputManager();

	// ���� ������ ���� ����
	static float shearSpeed = 2.f;
	static float scaleMin = 5.f;
	static float scaleMax = 20.f;
	static float scaleSpeed = 20.f;
	static float rotateSpeed = 180.f;

	float deltaShear = input.GetAxis(InputAxis::XAxis) * shearSpeed * InDeltaSeconds;
	float deltaScale = input.GetAxis(InputAxis::ZAxis) * scaleSpeed * InDeltaSeconds;
	float deltaDegree = input.GetAxis(InputAxis::WAxis) * rotateSpeed * InDeltaSeconds;

	// ��ü�� ���� ���� ����
	currentShear += deltaShear;
	currentScale = Math::Clamp(currentScale + deltaScale, scaleMin, scaleMax);
	currentDegree += deltaDegree;
}

// ������ ������ ����ϴ� �Լ�
void SoftRenderer::Render2D()
{
	// ������ �������� ����ϴ� ��� �� �ֿ� ���۷���
	auto& r = GetRenderer();
	const auto& g = Get2DGameEngine();

	// ��濡 ���� �׸���
	DrawGizmo2D();

	// ������ ������ ���� ����
	float rad = 0.f;
	static float increment = 0.001f;
	static std::vector<Vector2> hearts;
	HSVColor hsv(0.f, 1.f, 0.85f);
	static Vector2 pivot(200.f, 0.f);

	// ��Ʈ�� �����ϴ� �� ����
	if (hearts.empty())
	{
		for (rad = 0.f; rad < Math::TwoPI; rad += increment)
		{
			float sin = sinf(rad);
			float cos = cosf(rad);
			float cos2 = cosf(2 * rad);
			float cos3 = cosf(3 * rad);
			float cos4 = cosf(4 * rad);
			float x = 16.f * sin * sin * sin;
			float y = 13 * cos - 5 * cos2 - 2 * cos3 - cos4;
			hearts.push_back(Vector2(x, y));
		}
	}
	
	// ũ�� ��ȯ���
	Vector2 sBasis1(currentScale, 0.f);
	Vector2 sBasis2(0.f, currentScale);
	Matrix2x2 sMatrix(sBasis1, sBasis2);

	// ȸ�� ��ȯ���
	float sin = 0.f, cos = 0.f;
	Math::GetSinCos(sin, cos, currentDegree);
	Vector2 rBasis1(cos, sin);
	Vector2 rBasis2(-sin, cos);
	Matrix2x2 rMatrix(rBasis1, rBasis2);

	// ���� ��ȯ���
	Vector2 shBasis1 = Vector2::UnitX;
	Vector2 shBasis2(currentShear, 1.f);
	Matrix2x2 shMatrix(shBasis1, shBasis2);

	// �ռ����
	Matrix2x2 cMatrix = shMatrix * rMatrix * sMatrix;

	// ũ�� ��ȯ����� �����
	float invScale = 1.f / currentScale;
	Vector2 isBasis1(invScale, 0.f);
	Vector2 isBasis2(0.f, invScale);
	Matrix2x2 isMatrix(isBasis1, isBasis2);

	// ȸ�� ��ȯ����� �����
	Matrix2x2 irMatrix = rMatrix.Transpose();

	// ���� ��ȯ����� �����
	Vector2 ishBasis1 = Vector2::UnitX;
	Vector2 ishBasis2(-currentShear, 1.f);
	Matrix2x2 ishMatrix(ishBasis1, ishBasis2);

	// ������� �ռ����(�������� �����ϱ�)
	Matrix2x2 icMatrix = isMatrix * irMatrix * ishMatrix;

	// �� ���� �ʱ�ȭ�� �� �����ϰ� ������Ű�鼭 ���� ���� ����
	rad = 0.f;
	for (auto const& v : hearts)
	{
		// ���� ��Ʈ(��ȯ��� �����ϱ�)
		Vector2 left = cMatrix * v;
		r.DrawPoint(left - pivot, hsv.ToLinearColor());

		// ������ ��Ʈ(���� ��Ʈ�� �߰��� ����� �����ϱ�)
		Vector2 right = icMatrix * left;
		r.DrawPoint(right + pivot, hsv.ToLinearColor());

		hsv.H = rad / Math::TwoPI;
		rad += increment;
	}

	// ���� �б�, ������, ȸ������ ȭ�鿡 ���
	r.PushStatisticText(std::string("Shear : ") + std::to_string(currentShear));
	r.PushStatisticText(std::string("Scale : ") + std::to_string(currentScale));
	r.PushStatisticText(std::string("Degree : ") + std::to_string(currentDegree));
}

// �޽ø� �׸��� �Լ�
void SoftRenderer::DrawMesh2D(const class DD::Mesh& InMesh, const Matrix3x3& InMatrix, const LinearColor& InColor)
{
}

// �ﰢ���� �׸��� �Լ�
void SoftRenderer::DrawTriangle2D(std::vector<DD::Vertex2D>& InVertices, const LinearColor& InColor, FillMode InFillMode)
{
}