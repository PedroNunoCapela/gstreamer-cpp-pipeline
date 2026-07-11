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
    void setUri();
    void connectSignals();
    static void padAddedHandler(GstElement *src, GstPad *pad, Pipeline *self);
    void onPadAdded(GstElement *src, GstPad *pad);

    // Pipeline elements
    GstElement *pipeline;
    GstElement *source;
    GstElement *convert;
    GstElement *sink;

    GstBus *bus;
};

#endif // PIPELINE_H