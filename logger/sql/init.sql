-- For uuid
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- Scheme
CREATE TABLE IF NOT EXISTS public.logs
(
    id uuid NOT NULL DEFAULT uuid_generate_v1(),
    created_on timestamp NOT NULL,
    component character varying(30) NOT NULL,
    category character varying(30) NOT NULL,
    lvl character varying(10) NOT NULL,
    message text NOT NULL,
    CONSTRAINT pk_logs_id PRIMARY KEY(id)
);

ALTER TABLE IF EXISTS public.logs
    OWNER to admin;

-- Index
DROP INDEX IF EXISTS log_index CASCADE;
CREATE INDEX log_index ON public.logs(created_on, component, category, lvl);

-- Custom type log_type for insert procedure
DROP TYPE IF EXISTS log_type CASCADE;
CREATE TYPE public.log_type AS (
    created_on timestamp,
    component character varying(30),
    category character varying(30),
    lvl character varying(10),
    message text
);

-- Insert procedure
CREATE OR REPLACE PROCEDURE public.add_logs(batch public.log_type[])
LANGUAGE SQL AS 
$$
    INSERT INTO public.logs(created_on, component, category, lvl, message)
    SELECT * FROM UNNEST(batch)
$$;