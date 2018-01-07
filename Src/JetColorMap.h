#ifndef _JETCOLORMAP_H_FILE_
#define _JETCOLORMAP_H_FILE_
// Jet colormap inspired by Matlab. Grayvalues are expected in the range [0, 1]
// and are converted to RGB values in the same range.
class JetColormap {
public:
	static float Red(const float gray) { return Base(gray - 0.25f); }
	static float Green(const float gray) { return Base(gray); }
	static float Blue(const float gray) { return Base(gray + 0.25f); }

private:
	static float Interpolate(const float val, const float y0, const float x0,
		const float y1, const float x1) {
		return (val - x0) * (y1 - y0) / (x1 - x0) + y0;
	}

	static float Base(const float val) {
		if (val <= 0.125f) {
			return 0.0f;
		}
		else if (val <= 0.375f) {
			return Interpolate(2.0f * val - 1.0f, 0.0f, -0.75f, 1.0f, -0.25f);
		}
		else if (val <= 0.625f) {
			return 1.0f;
		}
		else if (val <= 0.87f) {
			return Interpolate(2.0f * val - 1.0f, 1.0f, 0.25f, 0.0f, 0.75f);
		}
		else {
			return 0.0f;
		}
	}
};

#endif // !_JETCOLORMAP_H_FILE_
