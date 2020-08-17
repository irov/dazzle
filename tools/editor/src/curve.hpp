// [src] https://github.com/ocornut/imgui/issues/123
// [src] https://github.com/ocornut/imgui/issues/55

#pragma once

#include "imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <cmath>

/* To use, add this prototype somewhere..

namespace ImGui
{
    int Curve(const char *label, const ImVec2& size, int maxpoints, ImVec2 *points);
    float CurveValue(float p, int maxpoints, const ImVec2 *points);
};

*/
/*
    Example of use:

    ImVec2 foo[10];
    ...
    foo[0].x = -1; // init data so editor knows to take it from here
    ...
    if (ImGui::Curve("Das editor", ImVec2(600, 200), 10, foo))
    {
        // curve changed
    }
    ...
    float value_you_care_about = ImGui::CurveValue(0.7f, 10, foo); // calculate value at position 0.7
*/

namespace ImGui
{
    //////////////////////////////////////////////////////////////////////////
    float CurveValue( float _p, int _maxpoints, const ImVec2 * _points )
    {
        if( _maxpoints < 2 || _points == 0 )
            return 0;
        if( _p < 0 ) return _points[0].y;

        int left = 0;
        while( left < _maxpoints && _points[left].x < _p && _points[left].x != -1 ) left++;
        if( left ) left--;

        if( left == _maxpoints - 1 )
            return _points[_maxpoints - 1].y;

        float d = (_p - _points[left].x) / (_points[left + 1].x - _points[left].x);

        return _points[left].y + (_points[left + 1].y - _points[left].y) * d;
    }
    //////////////////////////////////////////////////////////////////////////
    int Curve( const char * _label, const ImVec2 & _size, const int _maxpoints, ImVec2 * _points, float _x_min = 0.f, float _x_max = 1.f, float _y_min = 0.f, float _y_max = 1.f )
    {
        int modified = 0;
        int i;
        if( _maxpoints < 2 || _points == 0 )
            return 0;

        if( _points[0].x < 0 )
        {
            _points[0].x = 0;
            _points[0].y = 0;
            _points[1].x = 1;
            _points[1].y = 1;
            _points[2].x = -1;
        }

        ImGuiWindow * window = GetCurrentWindow();
        ImGuiContext & g = *GImGui;
        const ImGuiStyle & style = g.Style;
        const ImGuiID id = window->GetID( _label );
        if( window->SkipItems )
            return 0;

        ImRect bb( window->DC.CursorPos, window->DC.CursorPos + _size );
        ItemSize( bb );
        if( !ItemAdd( bb, NULL ) )
            return 0;

        const bool hovered = ImGui::IsItemHovered();

        int max = 0;
        while( max < _maxpoints && _points[max].x >= 0 ) max++;

        RenderFrame( bb.Min, bb.Max, GetColorU32( ImGuiCol_FrameBg, 1 ), true, style.FrameRounding );

        float ht = bb.Max.y - bb.Min.y;
        float wd = bb.Max.x - bb.Min.x;

        const ImGuiID CURVE_ID_NONE = 0;
        const int CURVE_POINT_NONE = -1;

        static ImGuiID active_id = CURVE_ID_NONE;
        static int selected_point = CURVE_POINT_NONE;

        if( g.IO.MouseReleased[0] == true )
        {
            active_id = CURVE_ID_NONE;
            selected_point = CURVE_POINT_NONE;
        }

        bool isMoved = false;

        if( active_id == id && selected_point != CURVE_POINT_NONE )
        {
            modified = 1;
            ImVec2 pos = (g.IO.MousePos - bb.Min) / (bb.Max - bb.Min);
            pos.y = 1 - pos.y;

            if( pos.x < 0.f || selected_point == 0 )
            {
                pos.x = 0.f;
            }

            if( pos.x > 1.f || (max > 1 && selected_point == max - 1) )
            {
                pos.x = 1.f;
            }

            if( pos.y < 0.f )
            {
                pos.y = 0.f;
            }

            if( pos.y > 1.f )
            {
                pos.y = 1.f;
            }

            _points[selected_point] = pos;

            if( max > 2 )
            {
                if( selected_point - 1 > 0 && _points[selected_point].x < _points[selected_point - 1].x )
                {
                    _points[selected_point].x = _points[selected_point - 1].x;
                }
                else if( selected_point + 1 < max - 1 && _points[selected_point].x > _points[selected_point + 1].x )
                {
                    _points[selected_point].x = _points[selected_point + 1].x;
                }
            }
            
            isMoved = true;
        }
        else if( hovered && active_id == CURVE_ID_NONE )
        {
            SetHoveredID( id );
            if( g.IO.MouseDown[0] == true )
            {
                modified = 1;
                ImVec2 pos = (g.IO.MousePos - bb.Min) / (bb.Max - bb.Min);
                pos.y = 1 - pos.y;

                int left = 0;
                while( left < max && _points[left].x < pos.x ) left++;
                if( left ) left--;

                int sel = -1;

                if( selected_point == CURVE_POINT_NONE )
                {
                    ImVec2 p = _points[left] - pos;

                    if( max == 1 )
                    {
                        float p1d = abs( p.y );

                        if( p1d < (1.f / 16.f) ) sel = left;
                    }
                    else
                    {
                        float p1d = sqrt( p.x * p.x + p.y * p.y );
                        p = _points[left + 1] - pos;
                        float p2d = sqrt( p.x * p.x + p.y * p.y );

                        if( p1d < (1.f / 16.f) ) sel = left;
                        if( p2d < (1.f / 16.f) ) sel = left + 1;
                    }

                    active_id = id;
                    selected_point = sel;
                }
                else
                {
                    sel = selected_point;
                }

                if( sel != -1 )
                {
                    if( g.IO.KeyAlt && max > 1 )
                    {
                        // kill selected
                        modified = 1;
                        for( i = sel + 1; i < max; i++ )
                        {
                            _points[i - 1] = _points[i];
                        }
                        max--;
                        _points[max].x = -1;

                        active_id = CURVE_ID_NONE;
                        selected_point = CURVE_POINT_NONE;
                    }
                    else
                    {
                        _points[sel] = pos;
                        isMoved = true;
                    }
                }
                else if ( g.IO.KeyCtrl )
                {
                    // add new point
                    if( max < _maxpoints && max == 1 && _maxpoints > 2)
                    {
                        _points[1] = pos;
                        _points[2].x = 1.f;
                        _points[2].y = _points[0].y;

                        max = 3;
                    }
                    else if( max < _maxpoints )
                    {
                        max++;
                        for( i = max; i > left; i-- )
                        {
                            _points[i] = _points[i - 1];
                        }
                        _points[left + 1] = pos;
                    }
                    if( max < _maxpoints )
                        _points[max].x = -1;
                }

                // snap first/last to min/max
                if( _points[0].x < _points[max - 1].x )
                {
                    _points[0].x = 0.f;
                    _points[max - 1].x = 1.f;
                }
                else
                {
                    _points[0].x = 1.f;
                    _points[max - 1].x = 0.f;
                }
            }
        }

        // horizontal grid lines
        window->DrawList->AddLine(
            ImVec2( bb.Min.x, bb.Min.y + ht / 2.f ),
            ImVec2( bb.Max.x, bb.Min.y + ht / 2.f ),
            GetColorU32( ImGuiCol_TextDisabled ) );

        window->DrawList->AddLine(
            ImVec2( bb.Min.x, bb.Min.y + ht / 4.f ),
            ImVec2( bb.Max.x, bb.Min.y + ht / 4.f ),
            GetColorU32( ImGuiCol_TextDisabled ) );

        window->DrawList->AddLine(
            ImVec2( bb.Min.x, bb.Min.y + ht / 4.f * 3.f ),
            ImVec2( bb.Max.x, bb.Min.y + ht / 4.f * 3.f ),
            GetColorU32( ImGuiCol_TextDisabled ) );

        // vertical grid lines
        for( i = 0; i < 9; i++ )
        {
            window->DrawList->AddLine(
                ImVec2( bb.Min.x + (wd / 10.f) * (i + 1), bb.Min.y ),
                ImVec2( bb.Min.x + (wd / 10.f) * (i + 1), bb.Max.y ),
                GetColorU32( ImGuiCol_TextDisabled ) );
        }

        ImU32 lineColorIdle = GetColorU32( ImGuiCol_PlotHistogramHovered );
        //ImU32 lineColorSelected = GetColorU32( ImGuiCol_PlotLinesHovered );

        // lines
        if( max == 1 )  // draw line when 1 point
        {
            ImVec2 a( 0.f, _points[0].y );
            ImVec2 b( 1.f, _points[0].y );
            a.y = 1 - a.y;
            b.y = 1 - b.y;
            a = a * (bb.Max - bb.Min) + bb.Min;
            b = b * (bb.Max - bb.Min) + bb.Min;
            window->DrawList->AddLine( a, b, lineColorIdle );
        }
        else
        {
            for( i = 1; i < max; i++ )
            {
                ImVec2 a = _points[i - 1];
                ImVec2 b = _points[i];
                a.y = 1 - a.y;
                b.y = 1 - b.y;
                a = a * (bb.Max - bb.Min) + bb.Min;
                b = b * (bb.Max - bb.Min) + bb.Min;
                window->DrawList->AddLine( a, b, lineColorIdle );
            }

            //if( hovered )
            if( (active_id == CURVE_ID_NONE && hovered == true) || (active_id == id) )
            {
                // control points
                for( i = 0; i < max; i++ )
                {
                    ImVec2 p = _points[i];
                    p.y = 1.f - p.y;
                    p = p * (bb.Max - bb.Min) + bb.Min;
                    ImVec2 a = p - ImVec2( 2.f, 2.f );
                    ImVec2 b = p + ImVec2( 2.f, 2.f );
                    window->DrawList->AddRect( a, b, lineColorIdle );
                }
            }
        }

        ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetColorU32( ImGuiCol_Text, 0.7f ) );
        
        // labels
        {
            char buf[32];
            sprintf( buf, "%.2f", _y_min );

            RenderTextClipped( ImVec2( bb.Min.x, bb.Min.y + style.FramePadding.y ), bb.Max, buf, NULL, NULL, ImVec2( 0.f, 1.f ) );
        }

        {
            char buf[32];
            sprintf( buf, "%.2f", _y_max );

            RenderTextClipped( ImVec2( bb.Min.x, bb.Min.y + style.FramePadding.y ), bb.Max, buf, NULL, NULL, ImVec2( 0.f, 0.f ) );
        }

        {
            char buf[32];
            sprintf( buf, "%.2f", _x_max );

            RenderTextClipped( ImVec2( bb.Min.x, bb.Min.y + style.FramePadding.y ), bb.Max, buf, NULL, NULL, ImVec2( 1.f, 1.f ) );
        }

        // debug text
        //{
        //    char buf[256];
        //    sprintf( buf, "my_id=%d\nactive_id=%d\nsel=%d\nis_moved=%s"
        //        , id
        //        , active_id
        //        , selected_point 
        //        , isMoved == true ? "true" : "false"
        //    );

        //    RenderTextClipped( ImVec2( bb.Min.x, bb.Min.y + style.FramePadding.y ), bb.Max, buf, NULL, NULL, ImVec2( 0.5f, 0.5f ) );
        //}

        if( hovered )
        {
            ImVec2 pos = (g.IO.MousePos - bb.Min) / (bb.Max - bb.Min);
            pos.y = 1.f - pos.y;

            float x = 0.f;

            if( max > 1 )
            {
                x = _x_min + pos.x * (_x_max - _x_min);
            }

            float y = _y_min + pos.y * (_y_max - _y_min);

            ImGui::Text( "(%.3f,%.3f)", x, y );
        }
        else
        {
            ImGui::Text( "(0.000, 0.000)" );
        }

        ImGui::PopStyleColor( 1 );

        return modified;
    }

};