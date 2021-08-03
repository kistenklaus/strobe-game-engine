package org.strobe.gfx.camera;

import org.joml.Vector3f;

public record CameraFrustum(Vector3f nearBottomLeft, Vector3f nearTopLeft,
                            Vector3f nearTopRight, Vector3f nearBottomRight,
                            Vector3f farBottomLeft, Vector3f farTopLeft,
                            Vector3f farTopRight, Vector3f farBottomRight) {
}
