/*
    Copyright (c) 2007-2011 iMatix Corporation
    Copyright (c) 2007-2011 Other contributors as noted in the AUTHORS file

    This file is part of 0MQ.

    0MQ is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    0MQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __ZMQ_SESSION_HPP_INCLUDED__
#define __ZMQ_SESSION_HPP_INCLUDED__

#include "own.hpp"
#include "i_inout.hpp"
#include "io_object.hpp"
#include "blob.hpp"
#include "pipe.hpp"

namespace zmq
{

    class session_t :
        public own_t,
        public io_object_t,
        public i_inout,
        public i_pipe_events
    {
    public:

        session_t (class io_thread_t *io_thread_,
            class socket_base_t *socket_, const options_t &options_);

        //  To be used once only, when creating the session.
        void attach_pipe (class pipe_t *pipe_);

        //  i_inout interface implementation.
        bool read (msg_t *msg_);
        bool write (msg_t *msg_);
        void flush ();
        void detach ();

        //  i_pipe_events interface implementation.
        void read_activated (class pipe_t *pipe_);
        void write_activated (class pipe_t *pipe_);
        void hiccuped (class pipe_t *pipe_);
        void terminated (class pipe_t *pipe_);

    protected:

        //  Events from the engine. Attached is triggered when session is
        //  attached to a peer. The function can reject the new peer by
        //  returning false. Detached is triggered at the beginning of
        //  the termination process when session is about to be detached from
        //  the peer. If it returns false, session will be terminated.
        //  To be overloaded by the derived session type.
        virtual bool xattached (const blob_t &peer_identity_) = 0;
        virtual bool xdetached () = 0;

        //  Returns true if there is an engine attached to the session.
        bool has_engine ();

        //  Allows derives session types to (un)register session names.
        bool register_session (const blob_t &name_, class session_t *session_);
        void unregister_session (const blob_t &name_);

        ~session_t ();

    private:

        bool attached (const blob_t &peer_identity_);
        void detached ();

        //  Handlers for incoming commands.
        void process_plug ();
        void process_attach (struct i_engine *engine_,
            const blob_t &peer_identity_);
        void process_term (int linger_);

        //  i_poll_events handlers.
        void timer_event (int id_);

        //  Remove any half processed messages. Flush unflushed messages.
        //  Call this function when engine disconnect to get rid of leftovers.
        void clean_pipes ();

        //  Call this function to move on with the delayed process_term.
        void proceed_with_term ();

        //  Pipe connecting the session to its socket.
        class pipe_t *pipe;

        //  This flag is true if the remainder of the message being processed
        //  is still in the in pipe.
        bool incomplete_in;

        //  True if termination have been suspended to push the pending
        //  messages to the network.
        bool pending;

        //  The protocol I/O engine connected to the session.
        struct i_engine *engine;

        //  The socket the session belongs to.
        class socket_base_t *socket;

        //  I/O thread the session is living in. It will be used to plug in
        //  the engines into the same thread.
        class io_thread_t *io_thread;

        //  ID of the linger timer
        enum {linger_timer_id = 0x20};

        //  True is linger timer is running.
        bool has_linger_timer;

        session_t (const session_t&);
        const session_t &operator = (const session_t&);
    };

}

#endif
