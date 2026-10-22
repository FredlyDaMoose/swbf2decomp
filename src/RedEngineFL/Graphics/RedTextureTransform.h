#ifndef SWBF2_REDENGINEFL_GRAPHICS_REDTEXTURETRANSFORM_H
#define SWBF2_REDENGINEFL_GRAPHICS_REDTEXTURETRANSFORM_H

class RedTextureTransform
{
public:

    float operator()(int iRow, int iCol) const
    {
        if (m_bTransform)
        {
            return m_Transform[iRow][iCol];
        }
        return (iRow == iCol) ? 1.0f : 0.0f;
    }

    void SetScale(float fScaleX, float fScaleY)
    {
        if (!m_bTransform)
        {
            m_bTransform = 1;
            m_Transform[2][0] = 0.0f;
            m_Transform[2][1] = 0.0f;
        }
        m_Transform[0][0] = fScaleX;
        m_Transform[0][1] = 0.0f;
        m_Transform[1][0] = 0.0f;
        m_Transform[1][1] = fScaleY;
    }

    unsigned int m_bTransform;

    float m_Transform[3][2];
};
 

#endif
