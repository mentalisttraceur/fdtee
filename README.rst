fdtee
-----

``fdtee`` is like the well known program ``tee``, but with
a key difference: ``fdtee`` copies its ``stdin`` to any
file descriptor given, instead of to file names.


Why?
====

If you've ever found yourself asking:

    "How do I redirect to BOTH stdout and stderr?"

or

    "How do I pipe output to multiple processes (without using the
     non-standard/less-portable feature of process substitution)?"

then this tool is for you.

12345678901234567890123456789012345678901234567890123456789012345678901234567890
With ``fdtee``, to redirect the output of ``foo`` to both
``stdout`` and ``stderr`` (on most systems, file
descriptors 1 and 2 respectively), you would simply do:

.. code:: shell

    foo | fdtee 1 2

Or you can do however many file descriptors you want (in whatever order):

.. code:: shell

    foo | fdtee 1 4 2 5 3

just remember the rest of the shell command has to use redirection
to make use of the output on those file descriptors.


Why is ``tee`` not enough?
==========================

The common alternative for splitting output over both ``stdout``
and ``stderr`` is to use ``tee`` with virtual files:

.. code:: shell

    foo | tee /dev/stderr

But this is broken in several cases:

1. ``/dev/stderr`` is not guaranteed by POSIX, so it might not even exist.

2. Furthermore, using ``/dev/stderr`` (or ``/proc/self/fd/2``) can
   break when processes switch effective user IDs, are in a chroot,
   or otherwise lose permission or access to that path.

3. You may be using a Bourne/POSIX shell on Windows,
   or another OS without the same virtual files,
   and you don't have such files faked by the shell.

In all of those cases, ``tee /dev/stderr`` does not work.

More importantly, if you wanted to split your output over more than two streams
or to streams other than stdout and stderr, tee does not work as well. At best,
the syntax is messy because you'll need additional redirects and maybe more
than one instance of tee to pipe through, at worst, you simply can't, thanks
to the limitations above. There's also no standardized, consistently available
virtual files to use for redirecting to file descriptors greater than 2.


``fdtee``, from a design/architecture/philosophy standpoint
===========================================================

Depending on how one interprets the Unix philosophy, one
might say ``fdtee`` is what ``tee`` should've been.

``fdtee``, like ``tee``, does one thing and does it well
- repeats/copies its stdin to any file descriptor given.
It takes this philosophy to an even greater extent than
``tee`` does: it does not have special-case logic for 
``stdout``, neither by writing to ``stdout`` by default,
nor by needing a magic argument value (``-``) which is
parsed differently than the rest of its arguments.

Furthermore, when you combine ``fdtee`` with the file redirection
capabilities which existed in the Bourne shell since its early
days, you can do everything you can do with ``tee``, because this:

.. code:: shell

    foo | tee bar.txt

is equivalent to:

.. code:: shell

    foo | fdtee 1 3 3>bar.txt

And this:

.. code:: shell

    foo | tee -a bar.txt

is equivalent to:

.. code:: shell

    foo | fdtee 1 3 3>>bar.txt

So you could implement ``tee`` as a wrapper around ``fdtee``, 
but you cannot implement ``fdtee`` as a wrapper around
``tee`` in any portable manner. Thus, ``fdtee`` is
fundamentally a more flexible building block for doing
powerful things with a Bourne/POSIX shell than ``tee`` is.


Other Details
=============

Unlike ``tee``, ``fdtee`` does not write to ``stdout`` by default.

The primary reason for this is simple design simplicity/purity -
not treating ``stdout`` as a special case makes the code simpler.

Also, consider that you might want to redirect to several
file descriptors, but not to ``stdout``, for example:

.. code:: shell

    foo | fdtee 2 3

Having to write a 1 where you need it is a minimal cost, readable
and explicit, while the alternative (automatically outputing to
``stdout``) would've forced people to write code like this:

.. code:: shell

    foo | fdtee 2 3 1>/dev/null

or:

.. code:: shell

    foo | fdtee 2 1>&3

just to split the output of foo to file descriptors
2 (``stderr`` usually) and 3, which is worse.
