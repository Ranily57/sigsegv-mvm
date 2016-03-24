#include "mod.h"
#include "mod/util/overlay_shared.h"
#include "stub/usermessages_cl.h"
#include "util/float16.h"
#include "stub/cdebugoverlay.h"


enum OverlayType_t
{
	OVERLAY_BOX = 0,
	OVERLAY_SPHERE,
	OVERLAY_LINE,
	OVERLAY_TRIANGLE,
	OVERLAY_SWEPT_BOX,
	OVERLAY_BOX2,
};


struct OverlayBase_t
{
	OverlayType_t m_Type;
	int m_nCreationTick;
	int m_nServerCount;
	float m_flEndTime;
	OverlayBase_t *m_pNextOverlay;
};

struct OverlaySphere_t : public OverlayBase_t
{
	Vector vOrigin;
	float flRadius;
	int nTheta;
	int nPhi;
	int r;
	int g;
	int b;
	int a;
};


namespace Mod_Util_Overlay_Recv
{
	/* for OVERLAY_SPHERE, CDebugOverlay::DrawOverlay calls the IMaterial *
	 * version of RenderSphere, which is extremely ugly */
	DETOUR_DECL_STATIC(void, CDebugOverlay_DrawOverlay, OverlayBase_t *pOverlay)
	{
		if (pOverlay->m_Type == OVERLAY_SPHERE) {
			OverlaySphere_t *pSphere = static_cast<OverlaySphere_t *>(pOverlay);
			
			if (pSphere->a > 0) {
				RenderSphere(pSphere->vOrigin, pSphere->flRadius, pSphere->nTheta, pSphere->nPhi,
					Color(pSphere->r, pSphere->g, pSphere->b, pSphere->a), false);
			}
			
			RenderWireframeSphere(pSphere->vOrigin, pSphere->flRadius, pSphere->nTheta, pSphere->nPhi,
				Color(pSphere->r, pSphere->g, pSphere->b, pSphere->a), true);
		} else {
			DETOUR_STATIC_CALL(CDebugOverlay_DrawOverlay)(pOverlay);
		}
	}
	
	
	ConVar cvar_trace("sig_util_overlay_recv_trace", "0", FCVAR_NOTIFY,
		"Trace overlay usermessages as they are received");
	
	ConVar cvar_sphere_ntheta("sig_util_overlay_recv_ntheta", "16", FCVAR_NOTIFY,
		"Set nTheta value for CDebugOverlay::AddSphereOverlay");
	ConVar cvar_sphere_nphi("sig_util_overlay_recv_nphi", "16", FCVAR_NOTIFY,
		"Set nPhi value for CDebugOverlay::AddSphereOverlay");
	
	
	void Box(bf_read& msg)
	{
		Vector origin; msg.ReadBitVec3Coord(origin);
		Vector mins;   msg.ReadBitVec3Coord(mins);
		Vector maxs;   msg.ReadBitVec3Coord(maxs);
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		int a = msg.ReadByte();
		float flDuration; _float32(&flDuration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[Box] <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> %02x%02x%02x%02x %.2f\n",
				origin.x, origin.y, origin.z,
				mins.x, mins.y, mins.z,
				maxs.x, maxs.y, maxs.z,
				r, g, b, a,
				flDuration);
		}
		
		if (debugoverlay != nullptr) {
			debugoverlay->AddBoxOverlay(origin, mins, maxs, vec3_angle, r, g, b, a, flDuration);
		}
	}
	
	void BoxAngles(bf_read& msg)
	{
		Vector origin; msg.ReadBitVec3Coord(origin);
		Vector mins;   msg.ReadBitVec3Coord(mins);
		Vector maxs;   msg.ReadBitVec3Coord(maxs);
		QAngle angles; msg.ReadBitAngles(angles);
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		int a = msg.ReadByte();
		float flDuration; _float32(&flDuration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[BoxAngles] <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> %02x%02x%02x%02x %.2f\n",
				origin.x, origin.y, origin.z,
				mins.x, mins.y, mins.z,
				maxs.x, maxs.y, maxs.z,
				angles.x, angles.y, angles.z,
				r, g, b, a,
				flDuration);
		}
		
		if (debugoverlay != nullptr) {
			debugoverlay->AddBoxOverlay(origin, mins, maxs, angles, r, g, b, a, flDuration);
		}
	}
	
	void SweptBox(bf_read& msg)
	{
		Vector start;  msg.ReadBitVec3Coord(start);
		Vector end;    msg.ReadBitVec3Coord(end);
		Vector mins;   msg.ReadBitVec3Coord(mins);
		Vector maxs;   msg.ReadBitVec3Coord(maxs);
		QAngle angles; msg.ReadBitAngles(angles);
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		int a = msg.ReadByte();
		float flDuration; _float32(&flDuration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[SweptBox] <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> %02x%02x%02x%02x %.2f\n",
				start.x, start.y, start.z,
				end.x, end.y, end.z,
				mins.x, mins.y, mins.z,
				maxs.x, maxs.y, maxs.z,
				angles.x, angles.y, angles.z,
				r, g, b, a,
				flDuration);
		}
		
		if (debugoverlay != nullptr) {
			debugoverlay->AddSweptBoxOverlay(start, end, mins, maxs, angles, r, g, b, a, flDuration);
		}
	}
	
	void Line(bf_read& msg)
	{
		Vector origin; msg.ReadBitVec3Coord(origin);
		Vector target; msg.ReadBitVec3Coord(target);
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		bool noDepthTest = !!msg.ReadOneBit();
		float duration; _float32(&duration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[Line] <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> %02x%02x%02x %d %.2f\n",
				origin.x, origin.y, origin.z,
				target.x, target.y, target.z,
				r, g, b,
				noDepthTest,
				duration);
		}
		
		if (debugoverlay != nullptr) {
			debugoverlay->AddLineOverlay(origin, target, r, g, b, noDepthTest, duration);
		}
	}
	
	void Triangle(bf_read& msg)
	{
		Vector p1; msg.ReadBitVec3Coord(p1);
		Vector p2; msg.ReadBitVec3Coord(p2);
		Vector p3; msg.ReadBitVec3Coord(p3);
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		int a = msg.ReadByte();
		bool noDepthTest = !!msg.ReadOneBit();
		float duration; _float32(&duration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[Triangle] <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> %02x%02x%02x%02x %d %.2f\n",
				p1.x, p1.y, p1.z,
				p2.x, p2.y, p2.z,
				p3.x, p3.y, p3.z,
				r, g, b, a,
				noDepthTest,
				duration);
		}
		
		if (debugoverlay != nullptr) {
			debugoverlay->AddTriangleOverlay(p1, p2, p3, r, g, b, a, noDepthTest, duration);
		}
	}
	
	void EntityText(bf_read& msg)
	{
		int entityID = msg.ReadUBitLong(ENTITY_ID_BITS);
		int text_offset = msg.ReadChar();
		const char *text = msg.ReadAndAllocateString();
		float duration; _float32(&duration, msg.ReadWord());
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		int a = msg.ReadByte();
		
		if (cvar_trace.GetBool()) {
			DevMsg("[EntityText] #%d %d \"%s\" %.2f %02x%02x%02x%02x\n",
				entityID,
				text_offset,
				text,
				duration,
				r, g, b, a);
		}
		
		if (debugoverlay != nullptr) {
			debugoverlay->AddEntityTextOverlay(entityID, text_offset, duration, r, g, b, a, "%s", text);
		}
		
		delete[] text;
	}
	
	void EntityTextAtPosition(bf_read& msg)
	{
		Vector origin; msg.ReadBitVec3Coord(origin);
		int text_offset = msg.ReadChar();
		const char *text = msg.ReadAndAllocateString();
		float duration; _float32(&duration, msg.ReadWord());
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		int a = msg.ReadByte();
		
		if (cvar_trace.GetBool()) {
			DevMsg("[EntityTextAtPosition] <%+4.0f %+4.0f %+4.0f> %d \"%s\" %.2f %02x%02x%02x%02x\n",
				origin.x, origin.y, origin.z,
				text_offset,
				text,
				duration,
				r, g, b, a);
		}
		
		if (debugoverlay != nullptr) {
			debugoverlay->AddTextOverlayRGB(origin, text_offset, duration, r, g, b, a, "%s", text);
		}
		
		delete[] text;
	}
	
	void Grid(bf_read& msg)
	{
		Vector vPosition; msg.ReadBitVec3Coord(vPosition);
		
		if (cvar_trace.GetBool()) {
			DevMsg("[Grid] <%+4.0f %+4.0f %+4.0f>\n",
				vPosition.x, vPosition.y, vPosition.z);
		}
		
		if (debugoverlay != nullptr) {
			debugoverlay->AddGridOverlay(vPosition);
		}
	}
	
	void Text(bf_read& msg)
	{
		Vector origin; msg.ReadBitVec3Coord(origin);
		const char *text = msg.ReadAndAllocateString();
		bool bViewCheck = !!msg.ReadOneBit();
		float duration; _float32(&duration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[Text] <%+4.0f %+4.0f %+4.0f> \"%s\" %d %.2f\n",
				origin.x, origin.y, origin.z,
				text,
				bViewCheck,
				duration);
		}
		
		if (debugoverlay != nullptr) {
			debugoverlay->AddTextOverlay(origin, duration, "%s", text);
		}
		
		delete[] text;
	}
	
	void ScreenText(bf_read& msg)
	{
		float flXpos; _float32(&flXpos, msg.ReadWord());
		float flYpos; _float32(&flYpos, msg.ReadWord());
		const char *text = msg.ReadAndAllocateString();
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		int a = msg.ReadByte();
		float duration; _float32(&duration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[ScreenText] %5.1f %5.1f \"%s\" %02x%02x%02x%02x %.2f\n",
				flXpos, flYpos,
				text,
				r, g, b, a,
				duration);
		}
		
		if (debugoverlay != nullptr) {
			debugoverlay->AddScreenTextOverlay(flXpos, flYpos, duration, r, g, b, a, text);
		}
		
		delete[] text;
	}
	
	void Cross3D_ext(bf_read& msg)
	{
		Vector position; msg.ReadBitVec3Coord(position);
		Vector mins;     msg.ReadBitVec3Coord(mins);
		Vector maxs;     msg.ReadBitVec3Coord(maxs);
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		bool noDepthTest = !!msg.ReadOneBit();
		float flDuration; _float32(&flDuration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[Cross3D_ext] <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> %02x%02x%02x %d %.2f\n",
				position.x, position.y, position.z,
				mins.x, mins.y, mins.z,
				maxs.x, maxs.y, maxs.z,
				r, g, b,
				noDepthTest,
				flDuration);
		}
		
		if (debugoverlay != nullptr) {
			Vector start = mins + position;
			Vector end   = maxs + position;
			debugoverlay->AddLineOverlay(start, end, r, g, b, noDepthTest, flDuration);
			
			start.x += (maxs.x - mins.x);
			end.x   -= (maxs.x - mins.x);
			debugoverlay->AddLineOverlay(start, end, r, g, b, noDepthTest, flDuration);
			
			start.y += (maxs.y - mins.y);
			end.y   -= (maxs.y - mins.y);
			debugoverlay->AddLineOverlay(start, end, r, g, b, noDepthTest, flDuration);
			
			start.x -= (maxs.x - mins.x);
			end.x   += (maxs.x - mins.x);
			debugoverlay->AddLineOverlay(start, end, r, g, b, noDepthTest, flDuration);
		}
	}
	
	void Cross3D_size(bf_read& msg)
	{
		Vector position; msg.ReadBitVec3Coord(position);
		float size; _float32(&size, msg.ReadWord());
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		bool noDepthTest = !!msg.ReadOneBit();
		float flDuration; _float32(&flDuration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[Cross3D_size] <%+4.0f %+4.0f %+4.0f> %5.1f %02x%02x%02x %d %.2f\n",
				position.x, position.y, position.z,
				size,
				r, g, b,
				noDepthTest,
				flDuration);
		}
		
		if (debugoverlay != nullptr) {
			debugoverlay->AddLineOverlay(position + Vector(size, 0.0f, 0.0f), position - Vector(size, 0.0f, 0.0f), r, g, b, noDepthTest, flDuration);
			debugoverlay->AddLineOverlay(position + Vector(0.0f, size, 0.0f), position - Vector(0.0f, size, 0.0f), r, g, b, noDepthTest, flDuration);
			debugoverlay->AddLineOverlay(position + Vector(0.0f, 0.0f, size), position - Vector(0.0f, 0.0f, size), r, g, b, noDepthTest, flDuration);
		}
	}
	
	void Cross3DOriented_ang(bf_read& msg)
	{
		Vector position; msg.ReadBitVec3Coord(position);
		QAngle angles;   msg.ReadBitAngles(angles);
		float size; _float32(&size, msg.ReadWord());
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		bool noDepthTest = !!msg.ReadOneBit();
		float flDuration; _float32(&flDuration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[Cross3DOriented_ang] <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> %5.1f %02x%02x%02x %d %.2f\n",
				position.x, position.y, position.z,
				angles.x, angles.y, angles.z,
				size,
				r, g, b,
				noDepthTest,
				flDuration);
		}
		
		if (debugoverlay != nullptr) {
			Vector forward, right, up;
			AngleVectors(angles, &forward, &right, &up);
			
			forward *= size;
			right   *= size;
			up      *= size;
			
			debugoverlay->AddLineOverlay(position + right,   position - right,   r, g, b, noDepthTest, flDuration);
			debugoverlay->AddLineOverlay(position + forward, position - forward, r, g, b, noDepthTest, flDuration);
			debugoverlay->AddLineOverlay(position + up,      position - up,      r, g, b, noDepthTest, flDuration);
		}
	}
	
	void Do_HorzArrow(const Vector& startPos, const Vector& endPos, float width, int r, int g, int b, int a, bool noDepthTest, float flDuration)
	{
		Vector lineDir = (endPos - startPos);
		VectorNormalize(lineDir);
		Vector upVec = Vector(0.0f, 0.0f, 1.0f);
		Vector sideDir;
		float radius = width / 2.0f;
		
		CrossProduct(lineDir, upVec, sideDir);
		
		Vector p1 = startPos - (sideDir * radius);
		Vector p2 = endPos - (lineDir * width) - (sideDir * radius);
		Vector p3 = endPos - (lineDir * width) - (sideDir * width);
		Vector p4 = endPos;
		Vector p5 = endPos - (lineDir * width) + (sideDir * width);
		Vector p6 = endPos - (lineDir * width) + (sideDir * radius);
		Vector p7 = startPos + (sideDir * radius);
		
		debugoverlay->AddLineOverlay(p1, p2, r, g, b, noDepthTest, flDuration);
		debugoverlay->AddLineOverlay(p2, p3, r, g, b, noDepthTest, flDuration);
		debugoverlay->AddLineOverlay(p3, p4, r, g, b, noDepthTest, flDuration);
		debugoverlay->AddLineOverlay(p4, p5, r, g, b, noDepthTest, flDuration);
		debugoverlay->AddLineOverlay(p5, p6, r, g, b, noDepthTest, flDuration);
		debugoverlay->AddLineOverlay(p6, p7, r, g, b, noDepthTest, flDuration);
		
		if (a > 0) {
			debugoverlay->AddTriangleOverlay(p5, p4, p3, r, g, b, a, noDepthTest, flDuration);
			debugoverlay->AddTriangleOverlay(p1, p7, p6, r, g, b, a, noDepthTest, flDuration);
			debugoverlay->AddTriangleOverlay(p6, p2, p1, r, g, b, a, noDepthTest, flDuration);
			
			debugoverlay->AddTriangleOverlay(p3, p4, p5, r, g, b, a, noDepthTest, flDuration);
			debugoverlay->AddTriangleOverlay(p6, p7, p1, r, g, b, a, noDepthTest, flDuration);
			debugoverlay->AddTriangleOverlay(p1, p2, p6, r, g, b, a, noDepthTest, flDuration);
		}
	}
	
	void HorzArrow(bf_read& msg)
	{
		Vector startPos; msg.ReadBitVec3Coord(startPos);
		Vector endPos;   msg.ReadBitVec3Coord(endPos);
		float width; _float32(&width, msg.ReadWord());
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		int a = msg.ReadByte();
		bool noDepthTest = !!msg.ReadOneBit();
		float flDuration; _float32(&flDuration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[HorzArrow] <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> %4.1f %02x%02x%02x%02x %d %.2f\n",
				startPos.x, startPos.y, startPos.z,
				endPos.x, endPos.y, endPos.z,
				width,
				r, g, b, a,
				noDepthTest,
				flDuration);
		}
		
		if (debugoverlay != nullptr) {
			Do_HorzArrow(startPos, endPos, width, r, g, b, a, noDepthTest, flDuration);
		}
	}
	
	void YawArrow(bf_read& msg)
	{
		Vector startPos; msg.ReadBitVec3Coord(startPos);
		float yaw; _float32(&yaw, msg.ReadWord());
		float length; _float32(&length, msg.ReadWord());
		float width; _float32(&width, msg.ReadWord());
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		int a = msg.ReadByte();
		bool noDepthTest = !!msg.ReadOneBit();
		float flDuration; _float32(&flDuration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[YawArrow] <%+4.0f %+4.0f %+4.0f> %5.1f %5.1f %4.1f %02x%02x%02x%02x %d %.2f\n",
				startPos.x, startPos.y, startPos.z,
				yaw,
				length,
				width,
				r, g, b, a,
				noDepthTest,
				flDuration);
		}
		
		if (debugoverlay != nullptr) {
			Do_HorzArrow(startPos, startPos + (UTIL_YawToVector(yaw) * length), width, r, g, b, a, noDepthTest, flDuration);
		}
	}
	
	void VertArrow(bf_read& msg)
	{
		Vector startPos; msg.ReadBitVec3Coord(startPos);
		Vector endPos;   msg.ReadBitVec3Coord(endPos);
		float width; _float32(&width, msg.ReadWord());
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		int a = msg.ReadByte();
		bool noDepthTest = !!msg.ReadOneBit();
		float flDuration; _float32(&flDuration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[VertArrow] <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> %4.1f %02x%02x%02x%02x %d %.2f\n",
				startPos.x, startPos.y, startPos.z,
				endPos.x, endPos.y, endPos.z,
				width,
				r, g, b, a,
				noDepthTest,
				flDuration);
		}
		
		if (debugoverlay != nullptr) {
			Vector lineDir = (endPos - startPos);
			VectorNormalize(lineDir);
			Vector upVec;
			Vector sideDir;
			float radius = width / 2.0f;
			
			VectorVectors(lineDir, sideDir, upVec);
			
			Vector p1 = startPos - (upVec * radius);
			Vector p2 = endPos - (lineDir * width) - (upVec * radius);
			Vector p3 = endPos - (lineDir * width) - (upVec * width);
			Vector p4 = endPos;
			Vector p5 = endPos - (lineDir * width) + (upVec * width);
			Vector p6 = endPos - (lineDir * width) + (upVec * radius);
			Vector p7 = startPos + (upVec * radius);
			
			debugoverlay->AddLineOverlay(p1, p2, r, g, b, noDepthTest, flDuration);
			debugoverlay->AddLineOverlay(p2, p3, r, g, b, noDepthTest, flDuration);
			debugoverlay->AddLineOverlay(p3, p4, r, g, b, noDepthTest, flDuration);
			debugoverlay->AddLineOverlay(p4, p5, r, g, b, noDepthTest, flDuration);
			debugoverlay->AddLineOverlay(p5, p6, r, g, b, noDepthTest, flDuration);
			debugoverlay->AddLineOverlay(p6, p7, r, g, b, noDepthTest, flDuration);
			
			if (a > 0) {
				debugoverlay->AddTriangleOverlay(p5, p4, p3, r, g, b, a, noDepthTest, flDuration);
				debugoverlay->AddTriangleOverlay(p1, p7, p6, r, g, b, a, noDepthTest, flDuration);
				debugoverlay->AddTriangleOverlay(p6, p2, p1, r, g, b, a, noDepthTest, flDuration);
				
				debugoverlay->AddTriangleOverlay(p3, p4, p5, r, g, b, a, noDepthTest, flDuration);
				debugoverlay->AddTriangleOverlay(p6, p7, p1, r, g, b, a, noDepthTest, flDuration);
				debugoverlay->AddTriangleOverlay(p1, p2, p6, r, g, b, a, noDepthTest, flDuration);
			}
		}
	}
	
	void Axis(bf_read& msg)
	{
		Vector position; msg.ReadBitVec3Coord(position);
		QAngle angles;   msg.ReadBitAngles(angles);
		float size; _float32(&size, msg.ReadWord());
		bool noDepthTest = !!msg.ReadOneBit();
		float flDuration; _float32(&flDuration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[Axis] <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> %4.1f %d %.2f\n",
				position.x, position.y, position.z,
				angles.x, angles.y, angles.z,
				size,
				noDepthTest,
				flDuration);
		}
		
		if (debugoverlay != nullptr) {
			Vector xvec, yvec, zvec;
			AngleVectors(angles, &xvec, &yvec, &zvec);
			
			xvec = position + (size * xvec);
			yvec = position - (size * yvec);
			zvec = position + (size * zvec);
			
			debugoverlay->AddLineOverlay(position, xvec, 255, 0, 0, noDepthTest, flDuration);
			debugoverlay->AddLineOverlay(position, yvec, 0, 255, 0, noDepthTest, flDuration);
			debugoverlay->AddLineOverlay(position, zvec, 0, 0, 255, noDepthTest, flDuration);
		}
	}
	
	void Sphere(bf_read& msg)
	{
		Vector center; msg.ReadBitVec3Coord(center);
		float radius; _float32(&radius, msg.ReadWord());
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		bool noDepthTest = !!msg.ReadOneBit();
		float flDuration; _float32(&flDuration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[Sphere] <%+4.0f %+4.0f %+4.0f> %5.1f %02x%02x%02x %d %.2f\n",
				center.x, center.y, center.z,
				radius,
				r, g, b,
				noDepthTest,
				flDuration);
		}
		
		if (debugoverlay != nullptr) {
			CDebugOverlay::AddSphereOverlay(center, radius, cvar_sphere_ntheta.GetInt(), cvar_sphere_nphi.GetInt(),
				r, g, b, 0x00, flDuration);
		}
		
#if 0
		if (debugoverlay != nullptr) {
			Vector edge, lastEdge;
			
			float axisSize = radius;
			debugoverlay->AddLineOverlay(center + Vector(0.0f, 0.0f, -axisSize), center + Vector(0.0f, 0.0f, axisSize), r, g, b, noDepthTest, flDuration);
			debugoverlay->AddLineOverlay(center + Vector(0.0f, -axisSize, 0.0f), center + Vector(0.0f, axisSize, 0.0f), r, g, b, noDepthTest, flDuration);
			debugoverlay->AddLineOverlay(center + Vector(-axisSize, 0.0f, 0.0f), center + Vector(axisSize, 0.0f, 0.0f), r, g, b, noDepthTest, flDuration);
			
			for (float angle = 0.0f; angle <= 360.0f; angle += 22.5f) {
				edge.x = center.x + (radius * cosf(angle / 180.0f * M_PI));
				edge.y = center.y;
				edge.z = center.z + (radius * sinf(angle / 180.0f * M_PI));
				
				if (angle != 0.0f) debugoverlay->AddLineOverlay(edge, lastEdge, r, g, b, noDepthTest, flDuration);
				
				lastEdge = edge;
			}
			
			for (float angle = 0.0f; angle <= 360.0f; angle += 22.5f) {
				edge.x = center.x;
				edge.y = center.y + (radius * cosf(angle / 180.0f * M_PI));
				edge.z = center.z + (radius * sinf(angle / 180.0f * M_PI));
				
				if (angle != 0.0f) debugoverlay->AddLineOverlay(edge, lastEdge, r, g, b, noDepthTest, flDuration);
				
				lastEdge = edge;
			}
			
			for (float angle = 0.0f; angle <= 360.0f; angle += 22.5f) {
				edge.x = center.x + (radius * cosf(angle / 180.0f * M_PI));
				edge.y = center.y + (radius * sinf(angle / 180.0f * M_PI));
				edge.z = center.z;
				
				if (angle != 0.0f) debugoverlay->AddLineOverlay(edge, lastEdge, r, g, b, noDepthTest, flDuration);
				
				lastEdge = edge;
			}
		}
#endif
	}
	
	void Do_Circle(const Vector& position, const Vector& xAxis, const Vector& yAxis, float radius, int r, int g, int b, int a, bool bNoDepthTest, float flDuration)
	{
		constexpr int nSegments = 16;
		constexpr float flRadStep = (M_PI * 2.0f) / (float)nSegments;
		
		Vector vecLastPosition;
		
		Vector vecStart = position + (radius * xAxis);
		Vector vecPosition = vecStart;
		
		for (int i = 1; i <= nSegments; ++i) {
			vecLastPosition = vecPosition;
			
			float flSin, flCos;
			SinCos(i * flRadStep, &flSin, &flCos);
			vecPosition = position + (radius * flCos * xAxis) + (radius * flSin * yAxis);
			
			debugoverlay->AddLineOverlay(vecLastPosition, vecPosition, r, g, b, bNoDepthTest, flDuration);
			
			if (a > 0 && i > 1) {
				debugoverlay->AddTriangleOverlay(vecStart, vecLastPosition, vecPosition, r, g, b, a, bNoDepthTest, flDuration);
				debugoverlay->AddTriangleOverlay(vecPosition, vecLastPosition, vecStart, r, g, b, a, bNoDepthTest, flDuration);
			}
		}
	}
	
	void Circle_ang(bf_read& msg)
	{
		Vector position; msg.ReadBitVec3Coord(position);
		QAngle angles;   msg.ReadBitAngles(angles);
		float radius; _float32(&radius, msg.ReadWord());
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		int a = msg.ReadByte();
		bool bNoDepthTest = !!msg.ReadOneBit();
		float flDuration; _float32(&flDuration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[Circle_ang] <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> %5.1f %02x%02x%02x%02x %d %.2f\n",
				position.x, position.y, position.z,
				angles.x, angles.y, angles.z,
				radius,
				r, g, b, a,
				bNoDepthTest,
				flDuration);
		}
		
		if (debugoverlay != nullptr) {
			matrix3x4_t xform;
			AngleMatrix(angles, position, xform);
			Vector xAxis, yAxis;
			MatrixGetColumn(xform, 2, xAxis);
			MatrixGetColumn(xform, 1, yAxis);
			
			Do_Circle(position, xAxis, yAxis, radius, r, g, b, a, bNoDepthTest, flDuration);
		}
	}
	
	void Circle_axes(bf_read& msg)
	{
		Vector position; msg.ReadBitVec3Coord(position);
		Vector xAxis;    msg.ReadBitVec3Coord(xAxis);
		Vector yAxis;    msg.ReadBitVec3Coord(yAxis);
		float radius; _float32(&radius, msg.ReadWord());
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		int a = msg.ReadByte();
		bool bNoDepthTest = !!msg.ReadOneBit();
		float flDuration; _float32(&flDuration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[Circle_axes] <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> %5.1f %02x%02x%02x%02x %d %.2f\n",
				position.x, position.y, position.z,
				xAxis.x, xAxis.y, xAxis.z,
				yAxis.x, yAxis.y, yAxis.z,
				radius,
				r, g, b, a,
				bNoDepthTest,
				flDuration);
		}
		
		if (debugoverlay != nullptr) {
			Do_Circle(position, xAxis, yAxis, radius, r, g, b, a, bNoDepthTest, flDuration);
		}
	}
	
	void Sphere_ang(bf_read& msg)
	{
		Vector position; msg.ReadBitVec3Coord(position);
		QAngle angles;   msg.ReadBitAngles(angles);
		float radius; _float32(&radius, msg.ReadWord());
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		int a = msg.ReadByte();
		bool bNoDepthTest = !!msg.ReadOneBit();
		float flDuration; _float32(&flDuration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[Sphere_ang] <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> %5.1f %02x%02x%02x%02x %d %.2f\n",
				position.x, position.y, position.z,
				angles.x, angles.y, angles.z,
				radius,
				r, g, b, a,
				bNoDepthTest,
				flDuration);
		}
		
		if (debugoverlay != nullptr) {
			CDebugOverlay::AddSphereOverlay(position, radius, cvar_sphere_ntheta.GetInt(), cvar_sphere_nphi.GetInt(),
				r, g, b, a, flDuration);
		}
		
#if 0
		if (debugoverlay != nullptr) {
			matrix3x4_t xform;
			AngleMatrix(angles, position, xform);
			Vector xAxis, yAxis, zAxis;
			MatrixGetColumn(xform, 0, xAxis);
			MatrixGetColumn(xform, 1, yAxis);
			MatrixGetColumn(xform, 2, zAxis);
			
			Do_Circle(position, xAxis, yAxis, radius, r, g, b, a, bNoDepthTest, flDuration);
			Do_Circle(position, yAxis, zAxis, radius, r, g, b, a, bNoDepthTest, flDuration);
			Do_Circle(position, xAxis, zAxis, radius, r, g, b, a, bNoDepthTest, flDuration);
		}
#endif
	}
	
	
	void Clear(bf_read& msg)
	{
		if (cvar_trace.GetBool()) {
			DevMsg("[Clear]\n");
		}
		
		if (debugoverlay != nullptr) {
			debugoverlay->ClearAllOverlays();
		}
	}
	
	void LineAlpha(bf_read& msg)
	{
		Vector origin; msg.ReadBitVec3Coord(origin);
		Vector target; msg.ReadBitVec3Coord(target);
		int r = msg.ReadByte();
		int g = msg.ReadByte();
		int b = msg.ReadByte();
		int a = msg.ReadByte();
		bool noDepthTest = !!msg.ReadOneBit();
		float flDuration; _float32(&flDuration, msg.ReadWord());
		
		if (cvar_trace.GetBool()) {
			DevMsg("[LineAlpha] <%+4.0f %+4.0f %+4.0f> <%+4.0f %+4.0f %+4.0f> %02x%02x%02x%02x %d %.2f\n",
				origin.x, origin.y, origin.z,
				target.x, target.y, target.z,
				r, g, b, a,
				noDepthTest,
				flDuration);
		}
		
		if (debugoverlay != nullptr) {
			debugoverlay->AddLineOverlayAlpha(origin, target, r, g, b, a, noDepthTest, flDuration);
		}
	}
	
	
	void BandwidthTest(bf_read& msg)
	{
		int bits = msg.ReadWord();
		
		if (cvar_trace.GetBool()) {
			DevMsg("[BandwidthTest] %d bits\n",
				bits);
		}
		
		msg.SeekRelative(bits);
	}
	
	
	void Hook_Overlays(bf_read& msg)
	{
//		while (msg.GetNumBitsLeft() > 0) {
			switch (msg.ReadUBitLong(OVERLAY_TYPE_BITS)) {
				
			case OV_BOX:                     Box(msg);                  break;
			case OV_BOX_ANGLES:              BoxAngles(msg);            break;
			case OV_SWEPT_BOX:               SweptBox(msg);             break;
			case OV_LINE:                    Line(msg);                 break;
			case OV_TRIANGLE:                Triangle(msg);             break;
			case OV_ENTITY_TEXT:             EntityText(msg);           break;
			case OV_ENTITY_TEXT_AT_POSITION: EntityTextAtPosition(msg); break;
			case OV_GRID:                    Grid(msg);                 break;
			case OV_TEXT:                    Text(msg);                 break;
			case OV_SCREEN_TEXT:             ScreenText(msg);           break;
			case OV_CROSS3D_EXT:             Cross3D_ext(msg);          break;
			case OV_CROSS3D_SIZE:            Cross3D_size(msg);         break;
			case OV_CROSS3D_ORIENTED_ANG:    Cross3DOriented_ang(msg);  break;
			case OV_HORZ_ARROW:              HorzArrow(msg);            break;
			case OV_YAW_ARROW:               YawArrow(msg);             break;
			case OV_VERT_ARROW:              VertArrow(msg);            break;
			case OV_AXIS:                    Axis(msg);                 break;
			case OV_SPHERE:                  Sphere(msg);               break;
			case OV_CIRCLE_ANG:              Circle_ang(msg);           break;
			case OV_CIRCLE_AXES:             Circle_axes(msg);          break;
			case OV_SPHERE_ANG:              Sphere_ang(msg);           break;
			case OV_CLEAR:                   Clear(msg);                break;
			case OV_LINE_ALPHA:              LineAlpha(msg);            break;
			case OV_BANDWIDTH_TEST:          BandwidthTest(msg);        break;
				
			default:
				Warning("Unknown overlay type %d\n");
				break;
			}
//		}
	}
	
	
	class CMod : public IMod
	{
	public:
		CMod() : IMod("Util:Overlay_Recv")
		{
			MOD_ADD_DETOUR_STATIC(CDebugOverlay_DrawOverlay, "[client] CDebugOverlay::DrawOverlay");
		}
		
		virtual void OnUnload() override
		{
			this->SetEnabled(false);
		}
		
		void SetEnabled(bool enable)
		{
			this->ToggleAllDetours(enable);
			
			if (this->m_bEnabled != enable) {
				if (enable) {
					this->Enable();
				} else {
					this->Disable();
				}
				
				this->m_bEnabled = enable;
			}
		}
		
		bool IsEnabled() const { return this->m_bEnabled; }
		
	private:
		void Enable()
		{
			if (usermessages->LookupUserMessage("Overlays") == -1) {
				usermessages->Register("Overlays", -1);
			}
			
			usermessages->HookMessage("Overlays", &Hook_Overlays);
		}
		void Disable()
		{
			usermessages->UnHookMessage("Overlays", &Hook_Overlays);
		}
		
		bool m_bEnabled = false;
	};
	CMod s_Mod;
	
	
	ConVar cvar_enable("sig_util_overlay_recv", "0", FCVAR_NOTIFY,
		"Utility: overlay forwarding: client receive",
		[](IConVar *pConVar, const char *pOldValue, float flOldValue) {
			ConVarRef var(pConVar);
			s_Mod.SetEnabled(var.GetBool());
		});
}