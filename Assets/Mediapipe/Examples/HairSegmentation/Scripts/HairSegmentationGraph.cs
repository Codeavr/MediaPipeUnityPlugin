using Mediapipe;
using UnityEngine;

using GL = Mediapipe.GL;

public class HairSegmentationGraph : DemoGraph {
  [SerializeField] private bool useGPU = true;

  private const string hairMaskStream = "hair_mask";
  private OutputStreamPoller<GpuBuffer> hairMaskStreamPoller;
  private GpuBufferPacket hairMaskPacket;

  private GameObject annotation;

  void Awake() {
    annotation = GameObject.Find("MaskAnnotation");
  }

  public override Status StartRun(SidePacket sidePacket) {
    hairMaskStreamPoller = graph.AddOutputStreamPoller<GpuBuffer>(hairMaskStream).ConsumeValue();
    hairMaskPacket = new GpuBufferPacket();

    return graph.StartRun(sidePacket);
  }

  public override void RenderOutput(WebCamScreenController screenController, Color32[] pixelData) {
    var hairMask = FetchNextHairMask();
    var texture = screenController.GetScreen();

    texture.SetPixels32(pixelData);
    RenderAnnotation(screenController, hairMask);

    texture.Apply();
  }

  private ImageFrame FetchNextHairMask() {
    if (!hairMaskStreamPoller.Next(hairMaskPacket)) {
      Debug.LogWarning($"Failed to fetch next packet from {hairMaskStream}");
      return null;
    }

    ImageFrame outputFrame = null;

    var status = gpuHelper.RunInGlContext(() => {
      var gpuFrame = hairMaskPacket.GetValue();
      var gpuFrameFormat = gpuFrame.Format();
      var sourceTexture = gpuHelper.CreateSourceTexture(gpuFrame);

      outputFrame = new ImageFrame(
        gpuFrameFormat.ImageFormatFor(), gpuFrame.Width(), gpuFrame.Height(), ImageFrame.kGlDefaultAlignmentBoundary);

      gpuHelper.BindFramebuffer(sourceTexture);
      var info = gpuFrameFormat.GlTextureInfoFor(0);

      GL.ReadPixels(0, 0, sourceTexture.Width(), sourceTexture.Height(), info.glFormat, info.glType, outputFrame.PixelDataPtr());
      GL.Flush();

      sourceTexture.Release();

      return Status.Ok(false);
    });

    if (!status.IsOk()) {
      Debug.LogError(status.ToString());
    }

    return outputFrame;
  }

  private void RenderAnnotation(WebCamScreenController screenController, ImageFrame hairMask) {
    // NOTE: input image is flipped
    annotation.GetComponent<MaskAnnotationController>().Draw(screenController.GetScreen(), hairMask, new Color(0, 0, 255), true);
  }

  public override bool shouldUseGPU() {
    return useGPU;
  }
}
