#pragma once

#include <QColor>

namespace palette {

inline const QColor background{18, 18, 18, 235};
inline const QColor glow{122, 162, 247, 26};
inline const QColor text{240, 240, 242};
inline const QColor dim{139, 143, 154};
inline const QColor accent{122, 162, 247};
inline const QColor accentEnd{187, 154, 247};
inline const QColor danger{247, 118, 142};
inline const QColor ink{17, 18, 26};
inline const QColor hairline{255, 255, 255, 30};
inline const QColor track{255, 255, 255, 12};
inline const QColor digitKey{255, 255, 255, 20};
inline const QColor functionKey{255, 255, 255, 10};
inline const QColor operatorKey{122, 162, 247, 40};
inline const QColor clearKey{247, 118, 142, 40};

inline QColor withAlpha(QColor color, int alpha)
{
    color.setAlpha(alpha);
    return color;
}

} // namespace palette
