/* #pragma ident	"@(#)g_unseal.c	1.13	98/01/22 SMI" */

/*
 * Copyright 1996 by Sun Microsystems, Inc.
 * 
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Sun Microsystems not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission. Sun Microsystems makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 * 
 * SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 *  glue routine gss_unseal
 */

#include "mglueP.h"

static OM_uint32
gssint_unseal_iov_shim(OM_uint32 *minor_status,
		       gss_ctx_id_t context_handle,
		       gss_buffer_t input_message_buffer,
		       gss_buffer_t output_message_buffer,
		       int *conf_state,
		       gss_qop_t *qop_state)
{
    OM_uint32		status;
    gss_iov_buffer_desc	iov[2];

    iov[0].type = GSS_IOV_BUFFER_TYPE_STREAM;
    iov[0].flags = 0;
    iov[0].buffer = *input_message_buffer;

    iov[1].type = GSS_IOV_BUFFER_TYPE_DATA;
    iov[1].flags = GSS_IOV_BUFFER_FLAG_ALLOCATE;
    iov[1].buffer.value = NULL;
    iov[1].buffer.length = 0;

    status = gss_unwrap_iov(minor_status, context_handle, conf_state,
			    qop_state, iov, sizeof(iov)/sizeof(iov[0]));

    *output_message_buffer = iov[1].buffer;

    return status;
}

OM_uint32 KRB5_CALLCONV
gss_unseal (minor_status,
            context_handle,
            input_message_buffer,
            output_message_buffer,
            conf_state,
            qop_state)

OM_uint32 *		minor_status;
gss_ctx_id_t		context_handle;
gss_buffer_t		input_message_buffer;
gss_buffer_t		output_message_buffer;
int *			conf_state;
int *			qop_state;

{
/* EXPORT DELETE START */
    OM_uint32		status;
    gss_union_ctx_id_t	ctx;
    gss_mechanism	mech;

    if (minor_status != NULL)
	*minor_status = 0;

    if (output_message_buffer != GSS_C_NO_BUFFER) {
	output_message_buffer->length = 0;
	output_message_buffer->value = NULL;
    }

    if (minor_status == NULL)
	return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (context_handle == GSS_C_NO_CONTEXT)
	return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_NO_CONTEXT);

    if (input_message_buffer == GSS_C_NO_BUFFER ||
	GSS_EMPTY_BUFFER(input_message_buffer))

	return (GSS_S_CALL_INACCESSIBLE_READ);

    if (output_message_buffer == GSS_C_NO_BUFFER)
	return (GSS_S_CALL_INACCESSIBLE_WRITE);

    /*
     * select the approprate underlying mechanism routine and
     * call it.
     */

    ctx = (gss_union_ctx_id_t) context_handle;
    mech = gssint_get_mechanism (ctx->mech_type);

    if (mech) {
	if (mech->gss_unseal) {
	    status = mech->gss_unseal(
				      mech->context,
				      minor_status,
				      ctx->internal_ctx_id,
				      input_message_buffer,
				      output_message_buffer,
				      conf_state,
				      qop_state);
	    if (status != GSS_S_COMPLETE)
		map_error(minor_status, mech);
	} else if (mech->gss_unwrap_iov) {
	    status = gssint_unseal_iov_shim(minor_status,
					    ctx,
					    input_message_buffer,
					    output_message_buffer,
					    conf_state,
					    (gss_qop_t *)qop_state);
	} else
	    status = GSS_S_UNAVAILABLE;

	return(status);
    }

/* EXPORT DELETE END */

    return (GSS_S_BAD_MECH);
}

OM_uint32 KRB5_CALLCONV
gss_unwrap (minor_status,
            context_handle,
            input_message_buffer,
            output_message_buffer,
            conf_state,
            qop_state)

OM_uint32 *		minor_status;
gss_ctx_id_t		context_handle;
gss_buffer_t		input_message_buffer;
gss_buffer_t		output_message_buffer;
int *			conf_state;
gss_qop_t *		qop_state;

{
    return (gss_unseal(minor_status, (gss_ctx_id_t)context_handle,
		       (gss_buffer_t)input_message_buffer,
		       output_message_buffer, conf_state, (int *) qop_state));
}
