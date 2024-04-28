#include <gst/gst.h>

int main(int argc, char* argv[]) {
    GstElement* pipeline, * source, * capsfilter, * depayloader, * decoder, * converter, * sink;
    GstBus* bus;
    GstMessage* msg;
    GstStateChangeReturn ret;

    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create elements
    pipeline = gst_pipeline_new("pipeline");
    source = gst_element_factory_make("udpsrc", "source"); // "udpsrc" is a network source that reads UDP packets from the network. It can be combined with RTP depayloaders to implement RTP streaming.
    capsfilter = gst_element_factory_make("capsfilter", "capsfilter"); // "capsfilter is" the element does not modify data as such, but can enforce limitations on the data format.
    depayloader = gst_element_factory_make("rtph264depay", "depayloader"); // "rtph264depay" extracts H264 video from RTP packets (RFC 3984)
    decoder = gst_element_factory_make("avdec_h264", "decoder"); // "avdec_h264" apprpiate libav h264 decoder to H264 video. 
    converter = gst_element_factory_make("videoconvert", "converter"); 
    sink = gst_element_factory_make("autovideosink", "sink"); // "autovideosink" is a video sink that automatically detects an appropriate video sink to use

    // Check if all elements could be created
    if (!pipeline || !source || !capsfilter || !depayloader || !decoder || !converter || !sink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    // To connect the receiver to sender, adjust the source port of the video to sender port. 
    g_object_set(source, "port", 5000, NULL);

    // "max-threads " is maximum number of worker threads to spawn. (0 = auto)
    g_object_set(decoder, "max-threads", 0, NULL);

    g_object_set(sink, "sync", FALSE, NULL);

    // Create caps for RTP H264 stream
    // Without capsfilter payloader types which is here "rtph264depay" does not work!!
    GstCaps* caps = gst_caps_new_simple("application/x-rtp",
        "media", G_TYPE_STRING, "video",
        "clock-rate", G_TYPE_INT, 90000,
        "encoding-name", G_TYPE_STRING, "H264",
        "payload", G_TYPE_INT, 96,
        NULL);
    g_object_set(capsfilter, "caps", caps, NULL);
    gst_caps_unref(caps);

    // Add elements to pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, capsfilter, depayloader, decoder, converter, sink, NULL);

    // Link elements
    if (!gst_element_link_many(source, capsfilter, depayloader, decoder, converter, sink, NULL)) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // Start playing
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // Wait until error or EOS
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
        (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    // Free resources
    if (msg != NULL)
        gst_message_unref(msg);
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    return 0;
}
