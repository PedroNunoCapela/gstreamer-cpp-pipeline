#include "Pipeline.h"

constexpr const char* VIDEO_PATH = "assets/dolphins.MP4";

Pipeline::Pipeline(int argc, char **argv)
{
    gst_init(&argc, &argv);
}

/*
 * This destructor frees any unfreed resource
 */
Pipeline::~Pipeline()
{
    if (pipeline)
    {
        gst_element_set_state(pipeline, GST_STATE_NULL); // state should be null before we can free the pipeline
        gst_object_unref(pipeline);
    }
    if (bus)
        gst_object_unref(bus);
}

/*
 * This method creates every element needed for our pipeline to work
 * it also creates an empty pipeline
 * returns true/false according to its success
 */
bool Pipeline::createPipelineElements()
{
    source = gst_element_factory_make("uridecodebin", "source");
    convert = gst_element_factory_make("videoconvert", "convert");
    flipVideo = gst_element_factory_make("videoflip", "flip");
    sink = gst_element_factory_make("autovideosink", "sink");

    // create an empty pipeline
    pipeline = gst_pipeline_new("main-pipeline");

    // verify if there is any errors
    if (!source || !sink || !convert || !flipVideo)
    {
        g_printerr("Failed to create elements!\n");
        return false;
    }

    return true;
}

/*
 * This method does 2 things:
 * adds every created element to the pipeline (builds the pipeline)
 * links every element together
 * returns true/false according to its success
 */
bool Pipeline::buildPipeline()
{
    gst_bin_add_many(GST_BIN(pipeline), source, convert, flipVideo, sink, NULL);

    // link elements together in the pipeline and verify if there is any errors
    if (!gst_element_link_many(convert, flipVideo, sink, NULL))
    {
        g_printerr("Error linking elements on the pipeline.");
        return false;
    }

    return true;
}

/*
 * This method changes the pipeline state to start playing
 */
bool Pipeline::startPipeline()
{
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // check for errors
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr("Failed to set pipeline playing state.");
        return false;
    }

    return true;
}

/*
 * This method retrieves the pipeline's bus and analyses the message returned by gst_bus_timed_pop_filtered
 * we want to know if there was an error or if the stream ended (EOS)
 */
void Pipeline::listenToBus()
{
    bus = gst_element_get_bus(pipeline);
    gboolean terminate = false;

    do
    {
        GstMessage *msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                                     (GstMessageType)(GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

        // parse message
        if (msg != NULL)
        {
            GError *err;
            gchar *debug_info;

            switch (GST_MESSAGE_TYPE(msg))
            {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error(msg, &err, &debug_info);
                g_printerr("Error received from element %s: %s\n",
                           GST_OBJECT_NAME(msg->src), err->message);
                g_printerr("Debugging information: %s\n",
                           debug_info ? debug_info : "none");
                g_clear_error(&err);
                g_free(debug_info);
                terminate = TRUE;
                break;
            case GST_MESSAGE_EOS:
                g_print("End-Of-Stream reached.\n");
                terminate = TRUE;
                break;
            case GST_MESSAGE_STATE_CHANGED:
                // We are only interested in state-changed messages from the pipeline
                if (GST_MESSAGE_SRC(msg) == GST_OBJECT(pipeline))
                {
                    GstState old_state, new_state, pending_state;
                    gst_message_parse_state_changed(msg, &old_state, &new_state,
                                                    &pending_state);
                    g_print("Pipeline state changed from %s to %s:\n",
                            gst_state_get_name(old_state), gst_state_get_name(new_state));
                }
                break;
            default:
                // We should not reach here
                g_printerr("Unexpected message received.\n");
                break;
            }
            gst_message_unref(msg);
        }
    } while (!terminate);
}

/*
 * This method was created because of the hidden 'this' pointer
 * that is automatically passed as an argument when accessing the method from a class.
 * The callback set on the connect method uses the GStreamer C method that uses only two parameters
 * (Trampoline)
 */
void Pipeline::padAddedHandler(GstElement *src, GstPad *pad, Pipeline *self)
{
    self->onPadAdded(src, pad);
}

/*
 * This method is the real callback that handles the signal sent by the connectSignals() method
 */

void Pipeline::onPadAdded(GstElement *src, GstPad *new_pad)
{
    GstPad *sink_pad = gst_element_get_static_pad(convert, "sink");
    GstCaps *new_pad_caps = nullptr;
    GstStructure *new_pad_struct = nullptr;
    const gchar *new_pad_type = nullptr;

    // there's no need to link the pad if it is already linked
    if (gst_pad_is_linked(sink_pad))
    {
        g_print("Converter already linked; ignoring additional pad.\n");
        gst_object_unref(sink_pad);
        return;
    }

    new_pad_caps = gst_pad_get_current_caps(new_pad);
    new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
    new_pad_type = gst_structure_get_name(new_pad_struct);

    if (!g_str_has_prefix(new_pad_type, "video/x-raw"))
    {
        g_print("Ignoring non-video pad: %s\n", new_pad_type);
        if (new_pad_caps)
            gst_caps_unref(new_pad_caps);
        gst_object_unref(sink_pad);
        return;
    }
    if (GST_PAD_LINK_FAILED(gst_pad_link(new_pad, sink_pad)))
        g_print("Video pad link failed.\n");
    else
        g_print("Video pad linked.\n");

    if (new_pad_caps)
        gst_caps_unref(new_pad_caps);
    gst_object_unref(sink_pad);
}

/*
 * This method configures the properties of the elements that need to be configured
 */
void Pipeline::configureElements()
{
    // TODO: remove this local URI
    // set's the URI for the video file we want to use in source

    gchar *fileURI = gst_filename_to_uri(VIDEO_PATH,
                                              NULL);

    g_object_set(source, "uri", fileURI, NULL);

    g_object_set(flipVideo, "method", 1, NULL); // 1 = rotate 90° clockwise
}

/*
 * This method is the signal emitted by source to its respective callback
 */
void Pipeline::connectSignals()
{
    g_signal_connect(source, "pad-added", G_CALLBACK(&Pipeline::padAddedHandler), this);
}

/*
 * This method ensures the correct execution flow of the pipeline
 * it returns 0/-1 to pipeline_main according to its success
 */
int Pipeline::run()
{
    if (!createPipelineElements())
        return -1;
    if (!buildPipeline())
        return -1;
    configureElements();
    connectSignals();
    if (!startPipeline())
        return -1;
    listenToBus();
    return 0;
}