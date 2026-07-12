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

    // Pipeline elements
// Pipeline elements
    GstElement *pipeline = nullptr;
    GstElement *source = nullptr;
    GstElement *convert = nullptr;
    GstElement *flipVideo = nullptr;
    GstElement *sink = nullptr;

    GstBus *bus = nullptr;
};

#endif // PIPELINE_H