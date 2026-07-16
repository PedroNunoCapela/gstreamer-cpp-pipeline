#ifndef PIPELINE_H
#define PIPELINE_H

#include <gst/gst.h>

class Pipeline
{
public:
    Pipeline(int argc, char **argv);
    ~Pipeline();
    int run();

private:
    bool createPipelineElements();
    bool buildPipeline();
    bool startPipeline();
    void listenToBus();
    void configureElements();
    void connectSignals();
    static void padAddedHandler(GstElement *src, GstPad *pad, Pipeline *self);
    void onPadAdded(GstElement *src, GstPad *pad);
    bool linkElements();
    bool linkElementsManual();

    // Pipeline elements

    GstElement *pipeline = nullptr;
    GstElement *source = nullptr;
    GstElement *convert = nullptr;
    GstElement *flipVideo = nullptr;
    GstElement *tee = nullptr;

    // elements for video display
    GstElement *display_queue = nullptr;
    GstElement *display_sink = nullptr;

    // elements for video recording
    GstElement *record_queue = nullptr;
    GstElement *record_sink = nullptr;
    GstElement *record_convert = nullptr;
    GstElement *encoder = nullptr;
    GstElement *parser = nullptr;
    GstElement *muxer = nullptr;

    // Pads
    GstPad *tee_display_pad = nullptr;
    GstPad *tee_record_pad = nullptr;

    GstBus *bus = nullptr;
};

#endif // PIPELINE_H