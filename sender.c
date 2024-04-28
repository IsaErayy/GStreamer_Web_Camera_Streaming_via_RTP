#include <gst/gst.h>

int main(int argc, char* argv[]) {
    GstElement* pipeline, * source, * convert, * encoder, * payloader, * sink;
    GstBus* bus;
    GstMessage* msg;
    GstStateChangeReturn ret;

    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create the elements
    
    pipeline = gst_pipeline_new("pipeline"); // Creating a new pipeline 
    source = gst_element_factory_make("ksvideosrc", "source"); // For webcam I use "ksvideosrc" as video source
    convert = gst_element_factory_make("videoconvert", "convert"); // I use "videoconvert " to convert video frames
    encoder = gst_element_factory_make("x264enc", "encoder"); // I use "x264enc" encoder to encode video frames  
    payloader = gst_element_factory_make("rtph264pay", "payloader"); // I use "rtph264pay" payloader with respect to  Real-time Transport Protocol (RTP) streaming.
    sink = gst_element_factory_make("udpsink", "sink"); // I send the packets to sink through UDP protocol.

    

    // Check if all elements are created
    if (!pipeline || !source || !convert || !encoder || !payloader || !sink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    // Set properties for elements 
  
    // "do-stats" is enabling logging of statistics
    g_object_set(source, "do-stats", TRUE, NULL);

    // "tune" preset name for non-psychovisual tuning options."zerolatency" is choosen to have less latency in streaming
    // If "bitrate" is low then image quality of streaming can be worse, for otherwise, the process speed can be low.
    // "speed-preset" preset name for speed/quality tradeoff options (can affect decode compatibility - impose restrictions separately for your target decoder) 
    g_object_set(encoder, "tune", "zerolatency", "bitrate", 1000, "speed-preset", "superfast", NULL); 

    // Send SPS and PPS Insertion Interval in seconds (sprop parameter sets will be multiplexed in the data stream when detected.) (0 = disabled, -1 = send with every IDR frame) 
    g_object_set(payloader, "zero-latency","config-interval", 0, NULL);

    
    // "host" is the host/IP/Multicast group to send the packets to
    // "port" is the port to send the packets to
    g_object_set(sink, "host", "127.0.0.1", "port", 5000, NULL);

    // Build the pipeline
    // "gst_bin_add_many" is the funcction where we add GstElements to link them in pipeline 
    // "gst_element_link_many" is the function where we link the elements. The arrangement of the element is important. If there is any mistake in arrangement then pipeline will not be created
    gst_bin_add_many(GST_BIN(pipeline), source, convert, encoder, payloader, sink, NULL);
    if (gst_element_link_many(source, convert, encoder, payloader, sink, NULL) != TRUE) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // Set pipeline state to playing
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr("Unable to set the pipeline to the playing state.\n ");
        g_object_unref(pipeline);
        return -1;
    }
    // Wait until error or EOS
    // The GstBus is an object responsible for delivering GstMessage packets in a first-in first-out way from the streaming threads (see GstTask) to the application.
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    // Free resources
    if (msg != NULL)
        gst_message_unref(msg);
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}
